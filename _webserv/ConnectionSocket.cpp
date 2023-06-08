/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/08 20:11:35 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"

//*		Main Constructor
ConnectionSocket::ConnectionSocket(
								int sock_fd,
								const t_server_block& conf_server_block,
								const t_epoll_data& edata) :
	assigned_server(conf_server_block),
	edata(edata),
	sock_stream(std::ios_base::in | std::ios_base::out)
{
	this->sock_fd = sock_fd;
	this->status = e_READ_MODE;
	this->parse_mode = e_READ_HEADS;
	this->cur_body_size = 0;
	flag = 0;
}

const t_server_block&	ConnectionSocket::getAssignedServer( void ) const
{
	return (this->assigned_server);
}

//*		Main Functions
void	ConnectionSocket::parse_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""parse_line() called()""\033[0m" << std::endl;
	
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	if (
		false == sock_stream.str().empty() ||
		(NULL != eevent &&  eevent->events & EPOLLIN))
	{
		read_line();
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
		this->status = e_RESP_MODE;//this->resp_mode_switch();
		// std::cout << "\033[1m\033[31m""cur_line(body): ""\033[0m" << cur_line << std::endl;
		req["body"] = cur_line;
		cur_line.erase(0);
	}
}

void	ConnectionSocket::parse_header( void ) { //std::cout << "reading headers" << std::endl;
	
	if (std::string::npos != cur_line.find("\r\n")) {
		if ("\r\n" == cur_line) {
			if (req.end() != req.find("Content-Length"))
			{
				parse_mode = e_READ_BODY;
				cur_body_size = std::atol(req["Content-Length"].c_str());
			}
			else {
				// std::cout << "end of request" << std::endl;
				this->status = e_RESP_MODE;//this->resp_mode_switch();
			}
		}
		else {
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\r'),  cur_line.end());
			cur_line.erase(std::remove(cur_line.begin(), cur_line.end(), '\n'),  cur_line.end());
			if (req.empty()) {
				req["req_line"] = cur_line;
			}
			else {
				std::stringstream	str_stream(cur_line);
				std::string			key;
				std::string			value;
						
				// std::cout << "\033[1m\033[31m""cur_line: ""\033[0m" << cur_line << std::endl;
				std::getline(str_stream, key, ':');
				std::getline(str_stream, value);
				req[key] = value;
			}
		}
		cur_line.erase(0);
	}
	// std::cout << "reading headers---END" << std::endl;
}

// void	ConnectionSocket::resp_mode_switch( void ) {
// 	this->status = e_RESP_MODE;
// 	this->set_directives();
// }

void	ConnectionSocket::read_line( void ) { //std::cout  << std::endl << "\033[1m\033[32m""read_line() called()""\033[0m" << std::endl;

	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	if (NULL != eevent && (eevent->events & EPOLLIN)) {
		memset(rcv_buf, '\0', RCV_BUF_SIZE + 1);
		if (recv(sock_fd, rcv_buf, RCV_BUF_SIZE, 0) <= 0)
			throw (SockEof());
		sock_stream << rcv_buf;
	}
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

	std::streamsize		buf_len = sock_stream.str().length();//()->in_avail();
	std::streamsize		bytes_read;
	char				buf[buf_len + 1];

	memset(buf, '\0', buf_len + 1);
	sock_stream.read(buf, buf_len);
	bytes_read = sock_stream.gcount();
	cur_line += buf;
	cur_body_size -= bytes_read;
}

void	ConnectionSocket::read_header( void ) {
	
	std::string			line_read;
	bool				has_eol = (std::string::npos != sock_stream.str().find("\r\n"));

	std::getline(sock_stream, line_read);//line_read += '\0';
	cur_line += line_read;
	if (has_eol)
		cur_line += "\n";

	//*		debug
	std::string	debug_string = line_read;
	debug_string.erase(std::remove(debug_string.begin(), debug_string.end(), '\r'), debug_string.end());
	// std::cout << "line_read: " << debug_string << " has_eol: " << has_eol << std::endl;
}

// void	ConnectionSocket::set_directives( void ) {
	
// }

void	ConnectionSocket::print_req( void ) {
	std::cout << "| START print_req |" << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.begin(); it != req.end(); it++) {
		std::cout << "|" << it->first << " : " << it->second << "|" << std::endl;
	}
	std::cout << "| END print_req |" << std::endl;
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
