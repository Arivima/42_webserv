/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
# include "Utils.hpp"

#include <sys/socket.h>	//recv
#include <iostream>		//cout
#include <sstream>		//string stream
#include <string>		//stoi
#include <cstring>		//memset
#include <algorithm>	//std::remove, std::find

//*		main Constructors and Destructors

Request::Request(
	const int sock_fd,
	const t_epoll_data& edata,
	long long& cur_memory_usage
	) :
	sock_fd(sock_fd),
	edata(edata),
	cur_memory_usage(cur_memory_usage)
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

	clock_gettime(CLOCK_BOOTTIME, &timestamp_start);
	timed_out = false;
}

Request::~Request( void )
{
	for (std::map<std::string, std::string>::iterator it = req.begin(); it != req.end(); it++)
		cur_memory_usage -= ((*it).first.length() + (*it).second.length());
	req.clear();

	cur_memory_usage -= payload.size();
	payload.clear();

	memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
	
	cur_memory_usage -= sock_stream.size();
	sock_stream.clear();
	
	cur_memory_usage -= cur_line.size();
	cur_line.clear();
}


//*		Main Functions

std::vector<char>		Request::getIncomingData( void )
{ COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW "Request::getIncomingData()" RESET << std::endl);
	std::vector<char>				incomingData;
	std::vector<char>::iterator		cr_pos;
	std::vector<char>				line;
	std::stringstream				s_hex_s;

	COUT_DEBUG_INSERTION(FULL_DEBUG, "BEFORE\n");
	parse_line();
	if(payload.empty())
		throw (ChunkNotComplete());
	COUT_DEBUG_INSERTION(FULL_DEBUG, "AFTER\n");
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
			s_hex_s << std::hex << line.data();
			s_hex_s >> cur_chunk_size;
			s_hex_s.str("");
			COUT_DEBUG_INSERTION(FULL_DEBUG, "current chunk size : " << cur_chunk_size << std::endl);
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
			payload.begin() + cur_chunk_size + 2
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

bool						Request::isRequestTimeout( void ){
    struct timespec	timestamp_now;
	double			elapsed_secs;
	const double	timeout_value = DEFAULT_TIMEOUT;
	long			timestamp_now_us;
	long			timestamp_start_us;
	
	clock_gettime(CLOCK_BOOTTIME, &timestamp_now);
	timestamp_start_us	= (this->timestamp_start.tv_sec * pow(10, 9)) + this->timestamp_start.tv_nsec;
	timestamp_now_us	= (timestamp_now.tv_sec * pow(10, 9)) + timestamp_now.tv_nsec;
    elapsed_secs		= static_cast<double>(timestamp_now_us - timestamp_start_us) / pow(10, 9);
	
	return (elapsed_secs > timeout_value);
}

void	Request::parse_line( void )
{
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);
	
	if (this->isRequestTimeout()) {
		timed_out = true;
		req["method"] = "UNKNOWN";
		req["url"] = "/";
		req["http_version"] = "HTTP/1.1";
		throw TaskFulfilled();
	}
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
		if (cur_memory_usage >= MAX_MEMORY_USAGE)
		{
			timed_out = true;
			req["method"] = "UNKNOWN";
			req["url"] = "/";
			req["http_version"] = "HTTP/1.1";
			throw TaskFulfilled();
		}
		
		//*	Dumping into dynamic entity for character handling (stream)
		sock_stream.insert(
			sock_stream.end(),
			rcv_buf,
			rcv_buf + bytes_read
		);

		cur_memory_usage += bytes_read;
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
				if (0 == cur_body_size || chunked)
					throw TaskFulfilled();
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
				strip_trailing_and_leading_spaces(value);
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
	if (false == chunked && 0 >= cur_body_size)
	{
		throw TaskFulfilled();
	}
}



//*		helper functions

void	Request::print_req( void ) {
	if (DEBUG){
		std::cout	<< BOLDCYAN "\n\nNew Request ----------------- " << RESET 
					<< CYAN " | cli_socket: " << this->sock_fd << RESET
					<< " | body_lenght: " << payload.size()
					<< std::endl ;
		
		std::cout << BOLDCYAN "| HEADERS _________________________" RESET << std::endl ;
		for (std::map<std::string, std::string>::iterator it = req.begin(); it != req.end(); it++) {
			std::cout << CYAN "| " RESET << it->first << " : " << it->second << CYAN " |" RESET << std::endl ;
		}
		std::cout << CYAN "| END HEADERS _____________________" RESET << std::endl ;

		std::cout << BOLDCYAN "\n| BODY ____________________________ " << RESET << std::endl ;
		for (std::vector<char>::iterator it = payload.begin(); it != payload.end(); it++)
			std::cout <<  *it ;
		if (payload.size())
			std::cout << std::endl ;
		std::cout << CYAN "| END BODY ________________________" RESET << std::endl ;
		std::cout << CYAN "END REQUEST -----------------\n" RESET << std::endl ;		
	}
}

void	Request::printVectorChar(std::vector<char>& v, std::string name)
{
	if (FULL_DEBUG){
		if (v.empty())
			std::cout << name << " is empty !" << std::endl;
		else
		{
			std::cout << name << ": " << std::endl;
			for (std::vector<char>::iterator it = v.begin(); it != v.end(); it++)
			{
				if ((*it) == '\n')
					std::cout << RED "n" RESET;
				else
				if ((*it) == '\r')
					std::cout << RED "r" RESET;
				else
					std::cout << *it;
			}
			std::cout << std::endl;
		}
	}
}

bool	Request::timedOut( void )
{
	return (this->timed_out);
}
