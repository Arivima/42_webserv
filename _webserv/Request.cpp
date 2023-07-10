/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/14 17:19:26 by earendil          #+#    #+#             */
/*   Updated: 2023/07/10 14:37:00 by avilla-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/Request.hpp"
#include <sys/socket.h>	//recv
#include <iostream>		//cout
#include <sstream>		//string stream
#include <string>		//stol
#include <cstring>		//memset
#include <algorithm>	//std::remove, std::find

//*		main Constructors and Destructors

Request::Request(
	const int sock_fd,
	const t_epoll_data& edata
	) :
	sock_fd(sock_fd),
	edata(edata)
{
	parser_status = e_READING_HEADS;
	cur_body_size = 0;
	chunked = false;
	next_chunk_arrived = false;
	cur_chunk_size = 0;

	req.clear();
	payload.clear();
	sock_stream.clear();
	cur_line.clear();
}

Request::~Request( void ) {
	req.clear();
	payload.clear();
	memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
	sock_stream.clear();
	cur_line.clear();
}


//*		Main Functions

std::vector<char>		Request::getIncomingData( void )
{
	const struct epoll_event*		eevent = edata.getEpollEvent(this->sock_fd);
	std::vector<char>				incomingData;
	std::vector<char>::iterator		cr_pos;
	std::vector<char>				line;

	read_line();
	if (false == next_chunk_arrived)
	{
		if (hasHttpHeaderDelimiter(payload))
		{
			cr_pos = std::find(payload.begin(), payload.end(), '\r');
			//*	inserting size line line into 'line'
			line.insert(
				line.begin(),
				payload.begin(),
				cr_pos
			);
			payload.erase(payload.begin(), cr_pos + 2);//*taking line off the payload

			line.push_back('\0');
			cur_chunk_size = std::stoll(line);
			next_chunk_arrived = true;
		}
		throw (ChunkNotComplete());
	}
	else
	{
		if (payload.size() < cur_chunk_size + 2)
			throw (ChunkNotComplete());
		if (
			payload[cur_chunk_size] != '\r' ||
			payload[cur_chunk_size + 1] != '\n' 
		)
			throw std::invalid_argument("Invalid Chunk read");
		incomingData.clear();
		incomingData.insert(
			incomingData.begin(),
			payload.begin(),
			payload.begin() + cur_chunk_size
		);
		payload.erase(
			payload.begin(),
			payload.begin() + cur_chunk_size
		);
		next_chunk_arrived = false;

		return (incomingData);
	}
}

const std::map<std::string, std::string>&	Request::getRequest( void ) {
	return (this->req);
}

const std::vector<char>&	Request::getPayload( void ) {
	return (this->payload);
}

bool						Request::isChunked( void ) {
	return (this->chunked);
}

void	Request::parse_line( void )
{
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	if (
		false == sock_stream.empty() ||
		(NULL != eevent &&  eevent->events & EPOLLIN))
	{
		read_line();
		if (e_READING_HEADS == parser_status) {
			parse_header();
		}
		else
		if (e_READING_BODY == parser_status) {
			parse_body();
		}
	}
}

void	Request::read_line( void )
{
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);
	int							bytes_read;

	//*	Checking for incoming data and writing into statically allocated recv buffer
	if (NULL != eevent && (eevent->events & EPOLLIN))
	{
		memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
		bytes_read = recv(sock_fd, rcv_buf, RCV_BUF_SIZE, 0);
		if (bytes_read <= 0)
			throw (SockEof());
		
		//*	Dumping into dynamic entity for character handling (stream)
		sock_stream.insert(
			sock_stream.end(),
			rcv_buf,
			rcv_buf + bytes_read
		);
	}

	//*	Read characters from dynamic entity for character handling (stream)
	if (e_READING_HEADS == parser_status) {
		read_header();
	}
	else
	if (e_READING_BODY == parser_status) {
		read_body();
	}
}

void	Request::read_header( void )
{
	std::vector<char>::iterator		lf_pos;

	if (hasHttpHeaderDelimiter(sock_stream))
	{
		lf_pos = std::find(sock_stream.begin(), sock_stream.end(), '\n');
		cur_line.insert(
			cur_line.end(),
			sock_stream.begin(),
			lf_pos + 1
		);
		sock_stream.erase(sock_stream.begin(), lf_pos + 1);
	}
}

void	Request::read_body( void ) {

	std::size_t		bytes_read = sock_stream.size();

	cur_line.insert(
		cur_line.end(),
		sock_stream.begin(),
		sock_stream.end()
	);
	sock_stream.clear();
	if (false == chunked)
		cur_body_size -= bytes_read;
}

void	Request::parse_header( void )
{
	if (false ==  cur_line.empty()) {
		if (isHttpHeaderDelimiter(cur_line))
		{
			//*	end of headers, there may or not may be a body
			cur_line.clear();
			if (req.end() == req.find("method")) {
				throw SockEof();
			}
			else
			if (req.end() != req.find("Content-Length"))
			{
				parser_status = e_READING_BODY;
				cur_body_size = std::atol(req["Content-Length"].c_str());
			}
			else {
				if (chunked)
					parser_status = e_READING_BODY;
				throw TaskFulfilled();
			}
		}
		else {
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\r'),  cur_line.end());
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\n'),  cur_line.end());
			cur_line.push_back('\0');
			if (req.empty()) {
				//*		request is empty, first line is request line
				parse_req_line(cur_line);
			}
			else {
				//*		request is not empty, line is header line
				std::stringstream	str_stream(cur_line.data());
				std::string			key;
				std::string			value;
				
				std::getline(str_stream, key, ':');
				std::getline(str_stream, value);
				req[key] = value;
				if (
					"Transfer-Encoding" == key &&
					"chunked" == value
				)
					chunked = true;
			}
		}
		cur_line.clear();
	}
}

void	Request::parse_req_line( std::vector<char>& req_line ) {

	std::stringstream	reqline_stream(req_line.data());
	std::string			method;
	std::string			url;
	std::string			http_version;

	std::getline(reqline_stream, method, ' ');
	std::getline(reqline_stream, url, ' ');
	std::getline(reqline_stream, http_version);
	req["method"] = method;
	req["url"] = url;
	req["http_version"] = http_version;

	if (std::string::npos != req["url"].find("http://"))
		req["url"].substr(7);
}

void	Request::parse_body( void ) {
	
	payload.insert(
		payload.end(),
		cur_line.begin(),
		cur_line.end()
	);
	cur_line.clear();
	if (false == chunked && 0 == cur_body_size)
	{
		throw TaskFulfilled();
	}
}



//*		helper functions

void	Request::print_req( void ) {
	std::cout << "| START print_req |" << std::endl;

	std::cout << BOLDGREEN "PRINTING HEADERS" RESET << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.begin(); it != req.end(); it++) {
		std::cout << "|" << it->first << " : " << it->second << "|" << std::endl;
	}
	std::cout << GREEN "END---PRINTING HEADERS" RESET << std::endl;

	std::cout << BOLDGREEN "PRINTING body" RESET << std::endl;
	for (std::vector<char>::iterator it = payload.begin(); it != payload.end(); it++)
		std::cout << *it;
	std::cout << std::endl;
	std::cout << "body len : " << payload.size() << std::endl;
	std::cout << GREEN "END---PRINTING body" RESET << std::endl;

	std::cout << "| END print_req |" << std::endl;
}

void	Request::printVectorChar(std::vector<char>& v, std::string name)
{
	if (v.empty())
		std::cout << name << " is empty !" << std::endl;
	else
	{
		std::cout << name << ": " << std::endl;
		for (std::vector<char>::iterator it = v.begin(); it != v.end(); it++)
		{
			if ((*it) == '\n')
				std::cout << "n";
			if ((*it) == '\r')
				std::cout << "r";
			std::cout << *it ;
		}
		std::cout << std::endl;
	}
}
