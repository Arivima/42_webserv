/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/14 17:19:26 by earendil          #+#    #+#             */
/*   Updated: 2023/07/05 17:41:47 by mmarinel         ###   ########.fr       */
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
	edata(edata),
	sock_stream(std::ios_base::in | std::ios_base::out)
{
	parser_status = e_READING_HEADS;
	cur_body_size = 0;
}

Request::~Request( void ) {
	req.clear();
}


//*		Main Functions

const std::map<std::string, std::string>&	Request::getRequest( void ) {
	return (this->req);
}

void	Request::parse_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""parse_line() called()""\033[0m" << std::endl;
	
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	if (
		false == sock_stream.str().empty() ||
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

	//*	Checking for incoming data and writing into statically allocated recv buffer
	if (NULL != eevent && (eevent->events & EPOLLIN))
	{
		memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
		if (recv(sock_fd, rcv_buf, RCV_BUF_SIZE, 0) <= 0)
			throw (SockEof());
		
		//*		DEBUG
		std::string		debug_rcv_buf = rcv_buf;
		std::string		debug_cur_line = cur_line;

		std::replace(debug_rcv_buf.begin(), debug_rcv_buf.end(), '\r', 'r');
		std::replace(debug_rcv_buf.begin(), debug_rcv_buf.end(), '\n', 'n');
		std::replace(debug_cur_line.begin(), debug_cur_line.end(), '\r', 'r');
		std::replace(debug_cur_line.begin(), debug_cur_line.end(), '\n', 'n');
		// COUT_DEBUG_INSERTION(
		// 	YELLOW "Request::read_line()" RESET
		// 	<< " - received chars : |" << debug_rcv_buf << "|" << std::endl
		// 	<< " - cur line : |" << debug_cur_line << "|"  << std::endl
		// 	<< " - cur stream : " << sock_stream.str() << std::endl
		// );
		//*************************************************************
		
		//*	Dumping into dynamic entity for character handling (stream)
		sock_stream << rcv_buf;
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
{
	std::string			line_read;
	// bool				has_eol = (std::string::npos != sock_stream.str().find("\r\n"));

	std::getline(sock_stream, line_read);//line_read += '\0';
	cur_line += line_read;
	if (sock_stream.good())//(has_eol)
		cur_line += "\n";
	else
		sock_stream.clear();
}

void	Request::read_body( void ) {

	std::streamsize		buf_len = sock_stream.str().length();//()->in_avail();
	std::streamsize		bytes_read;
	char				buf[buf_len + 1];

	memset(buf, '\0', buf_len + 1);
	sock_stream.read(buf, buf_len);
	bytes_read = sock_stream.gcount();
	cur_line += buf;
	cur_body_size -= bytes_read;
}

void	Request::parse_header( void )
{ //std::cout << "reading headers" << std::endl;
	if (std::string::npos != cur_line.find("\r\n")) {
		if ("\r\n" == cur_line) {
			//*	end of headers, there may or not may be a body
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
				throw TaskFulfilled();
			}
		}
		else {
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\r'),  cur_line.end());
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\n'),  cur_line.end());
			if (req.empty()) {
				//*		request is empty, first line is request line
				parse_req_line(cur_line);
			}
			else {
				//*		request is not empty, line is header line
				std::stringstream	str_stream(cur_line);
				std::string			key;
				std::string			value;
				
				std::getline(str_stream, key, ':');
				std::getline(str_stream, value);
				req[key] = value;
			}
		}
		cur_line.erase(0);
	}
	// std::cout << "reading headers---END" << std::endl;
}

void	Request::parse_req_line( std::string& req_line ) {

	std::stringstream	reqline_stream(req_line);
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
	if (0 == cur_body_size) {
		req["body"] = cur_line;
		cur_line.erase(0);
		throw TaskFulfilled();
	}
}

void	Request::print_req( void ) {
	std::cout << "| START print_req |" << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.begin(); it != req.end(); it++) {
		std::cout << "|" << it->first << " : " << it->second << "|" << std::endl;
	}
	std::cout << "| END print_req |" << std::endl;
}
