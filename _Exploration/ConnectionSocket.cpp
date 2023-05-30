/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/30 14:06:02 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
#include "SocketStream.hpp"

ConnectionSocket::ConnectionSocket(int sock_fd) :
	stream_buf(SocketStreamBuf(sock_fd)),
	stream(std::istream(&this->stream_buf))
{
	this->sock_fd = sock_fd;
	this->_cur_req_parsed = false;
	this->_parse_body = false;
	this->cur_body_size = 0;
}

void	ConnectionSocket::parse_line( void )
{
	t_PARSE_RET			parse_ret;
	
	parse_ret = read_line();
	if (e_OK_PARSE == parse_ret)
	{
		std::stringstream	str_stream(this->cur_line);
		std::string			header, value;

		if (
			("\r\n" == this->cur_line && false == this->_parse_body) ||
			(0 >= this->cur_body_size && true == this->_parse_body))
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

ConnectionSocket::t_PARSE_RET	ConnectionSocket::read_line( void )
{
	std::string						line_read;
	size_t							cr_pos;
	std::streamsize					bytes_read;
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
			return (e_CONTINUE_PARSE);
		}
		return (e_OK_PARSE);
	}
}
