/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/02 12:19:30 by earendil         ###   ########.fr       */
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
	this->cur_body_size = 0;
	flag = 0;
}

//*		Main Functions
void	ConnectionSocket::parse_line( void )
{
	if (
		stream.rdbuf()->in_avail() > 0 ||
		NULL != edata.getEpollEvent(this->sock_fd))
	{
		read_line();
		if (stream.eof())
			throw (SockEof());
		if (e_READ_BODY == parse_mode) {
			if (0 == cur_body_size) {
				status = e_RESP_MODE;
				req["body"] = cur_line;
				cur_line.erase(0);
			}
		}
		else
		if (e_READ_HEADS == parse_mode) {
			if (std::string::npos != cur_line.find("\r\n")) {
				if ("\r\n" == cur_line) {
					if (req.end() != req.find("Content-Length"))
					{
						parse_mode = e_READ_BODY;
						cur_body_size = std::atol(req["Content-Length"].c_str());
					}
					else
						status = e_RESP_MODE;
				}
				else {
					std::stringstream	str_stream(cur_line);
					std::string			key;
					std::string			value;

					std::getline(str_stream, key, ':');
					std::getline(str_stream, value);
					req[key] = value;
				}
				cur_line.erase(0);
			}
		}
	}
}

void	ConnectionSocket::read_line( void )
{
	std::string	line_read;

	if (e_READ_HEADS == parse_mode) {
		bool 		has_eol = dynamic_cast<SocketStreamBuf *>(
			stream.rdbuf())->has_eol();

		std::getline(stream, line_read);
		cur_line += line_read;
		if (has_eol)
			cur_line += '\n';
	}
	else
	if (e_READ_BODY == parse_mode) {
		std::streamsize	buf_len = std::min(1024, cur_body_size);
		char			buf[buf_len + 1];
		std::streamsize	bytes_read;

		memset(buf, '\0', buf_len + 1);
		stream.read(buf, buf_len);
		bytes_read = stream.gcount();
		cur_line += buf;
		cur_body_size -= bytes_read;
	}
}

//*		Utilities

int	ConnectionSocket::getSockFD( void ) {
	return (this->sock_fd);
}

//*		CANONICAL FORM

ConnectionSocket::~ConnectionSocket( void )
{
}
