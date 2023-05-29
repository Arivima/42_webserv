/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:42:21 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/29 18:53:17 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETSTREAM_HPP
# define SOCKETSTREAM_HPP

# include <fstream>

#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_addr

class SocketStreamBuf : public std::streambuf {
private:
	static const int		bufferSize = 1024;
	int						socketFd;
	char					buffer[bufferSize];

public:
	explicit SocketStreamBuf(int socketFd);

protected:
	int_type underflow( void );
};

#endif