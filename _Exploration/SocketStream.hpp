/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:42:21 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/31 11:51:56 by earendil         ###   ########.fr       */
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

	class	SockEof : public std::exception
	{
		public:
			virtual const char*	what( void ) const throw();
	};

protected:
	int_type underflow( void );
};

#endif