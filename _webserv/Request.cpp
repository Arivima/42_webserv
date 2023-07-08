/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/14 17:19:26 by earendil          #+#    #+#             */
/*   Updated: 2023/07/08 09:13:35 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/Request.hpp"
#include <sys/socket.h>	//recv
#include <iostream>		//cout
#include <sstream>		//string stream
#include <cstring>		//memset
#include <algorithm>	//std::remove

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
	std::vector<char>	incomingData;

	read_line();
	incomingData = cur_line;
	cur_line.clear();

	return (incomingData);
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

void	Request::parse_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""parse_line() called()""\033[0m" << std::endl;
	
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
	// std::cout << std::endl;
}

void	Request::read_line( void )
{ //std::cout  << std::endl << "\033[1m\033[32m""read_line() called()""\033[0m" << std::endl;
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);
	int							bytes_read;

	//*	Checking for incoming data and writing into statically allocated recv buffer
	if (NULL != eevent && (eevent->events & EPOLLIN))
	{
		memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
		bytes_read = recv(sock_fd, rcv_buf, RCV_BUF_SIZE, 0);
		if (bytes_read <= 0)
			throw (SockEof());
		
		//*		DEBUG
		// std::string			debug_rcv_buf = rcv_buf;
		// std::string			debug_cur_line = cur_line.data() == NULL ? "" : cur_line.data();
		// std::vector<char>	debug_sockstream(sock_stream);

		// debug_sockstream.push_back('\0');
		// std::replace(debug_rcv_buf.begin(), debug_rcv_buf.end(), '\r', 'r');
		// std::replace(debug_rcv_buf.begin(), debug_rcv_buf.end(), '\n', 'n');
		// std::replace(debug_cur_line.begin(), debug_cur_line.end(), '\r', 'r');
		// std::replace(debug_cur_line.begin(), debug_cur_line.end(), '\n', 'n');
		// COUT_DEBUG_INSERTION(
		// 	YELLOW "Request::read_line()" RESET
		// 	<< " - received chars : |" << debug_rcv_buf << "|" << std::endl
		// 	<< " - cur line : |" << debug_cur_line << "|"  << std::endl
		// 	<< " - cur stream : " << debug_sockstream.data() << std::endl
		// );
		//*************************************************************
		
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
	// std::cout << std::endl;
}

void	Request::read_header( void )
{ //std::cout << MAGENTA "Request::read_header()" RESET << std::endl;
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
	else
	{
		cur_line.insert(
			cur_line.end(),
			sock_stream.begin(),
			sock_stream.end()
		);
		sock_stream.clear();
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
	cur_body_size -= bytes_read;
}

void	Request::parse_header( void )
{ //std::cout << YELLOW "Request::parse_header()" RESET << std::endl;
	if (hasHttpHeaderDelimiter(cur_line)) {
		if (isHttpHeaderDelimiter(cur_line)) {
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
				std::stringstream	str_stream(std::string(cur_line.data()));
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
	// std::cout << "reading headers---END" << std::endl;
}

void	Request::parse_req_line( std::vector<char>& req_line ) {

	std::stringstream	reqline_stream(std::string(req_line.data()));
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
	std::cout << "parsing body" << std::endl;
	if (0 == cur_body_size) {
		payload.insert(
			payload.begin(),
			cur_line.begin(),
			cur_line.end()
		);
		cur_line.clear();
		throw TaskFulfilled();
	}
}

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
