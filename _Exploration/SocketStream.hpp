/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:42:21 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/04 15:10:37 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETSTREAM_HPP
# define SOCKETSTREAM_HPP

# include <fstream>
# include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
# include <iostream>

class SocketStreamBuf : public std::streambuf {
private:
	//*	HTTP/1.1 specification (RFC 7230)
	//*
	//*	max request line (Method url protocol version)
	//*		8000 bytes
	//*	max header line	(user-agent, etc.)
	//*		4096 bytes
	static const int		bufferSize = 8001;
	int						socketFd;
	char					buffer[bufferSize];

public:
	//*		constructor
	explicit SocketStreamBuf(int socketFd);
	//*		main functionalities
	bool	has_eol( void );
protected:
	int_type underflow( void );
};

#endif