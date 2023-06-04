/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/04 15:40:13 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
#include "SocketStream.hpp"

//*		Main Constructor
ConnectionSocket::ConnectionSocket(int sock_fd, const t_epoll_data& edata) :
	edata(edata),
	stream_buf(SocketStreamBuf(sock_fd)),
	stream(&this->stream_buf)
{
	this->sock_fd = sock_fd;
	this->status = e_READ_MODE;
	this->parse_mode = e_READ_HEADS;
	this->cur_body_size = 0;
	flag = 0;
}

//*		Main Functions
void	ConnectionSocket::parse_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""parse_line() called()""\033[0m" << std::endl;
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	if (
		stream.rdbuf()->in_avail() > 0 ||
		(NULL != eevent &&  eevent->events & EPOLLIN))
	{
		read_line();
		if (stream.eof())
			throw (SockEof());
		if (e_READ_BODY == parse_mode) {
			parse_body();
		}
		else
		if (e_READ_HEADS == parse_mode) {
			parse_header();
		}
	}
	// std::cout << std::endl;
}

void	ConnectionSocket::parse_body( void ) {
	if (0 == cur_body_size) {
		status = e_RESP_MODE;
		std::cout << "\033[1m\033[31m""cur_line(body): ""\033[0m" << cur_line << std::endl;
		req["body"] = cur_line;
		cur_line.erase(0);
	}
}

void	ConnectionSocket::parse_header( void ) { std::cout << "reading headers" << std::endl;
	if (std::string::npos != cur_line.find("\r\n")) {
		if ("\r\n" == cur_line) {
			if (req.end() != req.find("Content-Length"))
			{
				parse_mode = e_READ_BODY;
				cur_body_size = std::atol(req["Content-Length"].c_str());
			}
			else {
				std::cout << "end of request" << std::endl;
				status = e_RESP_MODE;
			}
		}
		else {
			std::stringstream	str_stream(cur_line);
			std::string			key;
			std::string			value;
			
			std::cout << "\033[1m\033[31m""cur_line: ""\033[0m" << cur_line << std::endl;
			std::getline(str_stream, key, ':');
			std::getline(str_stream, value);
			req[key] = value;
		}
		cur_line.erase(0);
	}
	std::cout << "reading headers---END" << std::endl;
}

void	ConnectionSocket::read_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""read_line() called()""\033[0m" << std::endl;
	if (e_READ_HEADS == parse_mode) {
		read_header();
	}
	else
	if (e_READ_BODY == parse_mode) {
		read_body();
	}
	// std::cout << std::endl;
}

void	ConnectionSocket::read_body( void ) {

	std::streamsize	buf_len = std::min(MAX_HTTP_REQ_LINE_LEN, cur_body_size);
	char			buf[buf_len + 1];
	std::streamsize	bytes_read;
	
	memset(buf, '\0', buf_len + 1);
	stream.read(buf, buf_len);
	bytes_read = stream.gcount();
	cur_line += buf;
	cur_body_size -= bytes_read;
}

void	ConnectionSocket::read_header( void ) {
	
	std::string	line_read;
	bool		has_eol
		= dynamic_cast<SocketStreamBuf *>(stream.rdbuf())->has_eol();

	std::getline(stream, line_read);//line_read += '\0';
	cur_line += line_read;
	if (has_eol)
		cur_line += "\n";

	if (cur_line.length() > (req.end() != req.find("REQ_LINE") ?
		MAX_HTTP_REQ_LINE_LEN :
		MAX_HTTP_HEADER_LINE_LEN))
		throw (LongHeader());
	//*		debug
	std::string	debug_string = line_read;
	debug_string.erase(std::remove(debug_string.begin(), debug_string.end(), '\r'), debug_string.end());
	std::cout << "line_read: " << debug_string << " has_eol: " << has_eol << std::endl;
}

//*		Exceptions
const char*	ConnectionSocket::SockEof::what( void ) const throw() {
	return ("ConnectionSocket: reached eof");
}
const char*	ConnectionSocket::LongHeader::what( void ) const throw() {
	return ("ConnectionSocket: err: too long header");
}


//*		Utilities

int	ConnectionSocket::getSockFD( void ) {
	return (this->sock_fd);
}

ConnectionSocket::t_CLIENT_STATUS	ConnectionSocket::getStatus( void ) {
	return (this->status);
}

//*		CANONICAL FORM

ConnectionSocket::~ConnectionSocket( void )
{
}
