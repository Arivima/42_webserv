/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/31 16:25:19 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
#include "SocketStream.hpp"

// #include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
// #include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
// #include <arpa/inet.h>      // inet_addr
// #include <fcntl.h>          // fcntl

//*		Main Constructor
ConnectionSocket::ConnectionSocket(int sock_fd) :
	stream_buf(SocketStreamBuf(sock_fd)),
	stream(&this->stream_buf)
{
	this->sock_fd = sock_fd;
    // int flags = fcntl(sock_fd, F_GETFL, 0);
    // // if (-1 == flags){
    // //     // ftError("fcntl() failed");
    // // }
    // flags |= O_NONBLOCK;
    // // if (-1 == fcntl(sock_fd, F_SETFL, flags)){
    // //     // ftError("fcntl() failed");
    // // }
	this->_cur_req_parsed = false;
	this->_parse_body = false;
	this->cur_body_size = 0;
	flag = 0;
}

//*		Main Functions
void	ConnectionSocket::parse_line( void )
{
	t_PARSE_RET			parse_ret;
	
	try {
		parse_ret = read_line();
		if (e_OK_PARSE == parse_ret)
		{
			std::stringstream	str_stream(this->cur_line);
			std::string			header, value;

			if (
				("\r\n" == this->cur_line && false == this->_parse_body) ||
				(0 == this->cur_body_size && true == this->_parse_body))
			{
				this->_cur_req_parsed = true;
			}
			else if (false == this->_parse_body)
			{
				// if (std::string::npos == this->cur_line.find(":"))
				// 	throw (ParseError());
				std::getline(str_stream, header, ':');
				std::getline(str_stream, value);
				this->headers[header] = value;
			}
			this->cur_line.erase(0);
		}
	}
	catch (const ParseError& e) {
		std::cout << e.what() << std::endl;
		this->cur_line.erase(0);
	}
}

ConnectionSocket::t_PARSE_RET	ConnectionSocket::read_line( void )
{
	std::string						line_read;
	size_t							cr_pos;
	const size_t					buffer_size = std::min((size_t)1024, this->cur_body_size);
	char							buf[buffer_size];

	if (this->_parse_body)
	{
		memset(buf, '\0', buffer_size);
		this->stream.read(buf, buffer_size);
		this->body += buf;
		this->cur_body_size -= this->stream.gcount();

		if (this->cur_body_size <= 0)
			return (e_OK_PARSE);
		else
			return (e_CONTINUE_PARSE);
	}
	else
	{
		std::getline(this->stream, line_read, '\n');
		this->cur_line += line_read;
		
		cr_pos = line_read.find("\r\n");
		if (std::string::npos == cr_pos)
		{
			if (std::string::npos == line_read.find("\n"))
				return (e_CONTINUE_PARSE);
			else
				throw (ParseError());
		}
		return (e_OK_PARSE);
	}
}

//*		Utilities

int	ConnectionSocket::getSockFD( void ) {
	return (this->sock_fd);
}

const char*	ConnectionSocket::ParseError::what( void ) const throw()
{
	return ("err: http req line not valid");
}

//*		CANONICAL FORM

ConnectionSocket::~ConnectionSocket( void )
{
}
