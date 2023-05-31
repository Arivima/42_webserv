/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:49:33 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/31 12:49:14 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketStream.hpp"

#include <iostream>

SocketStreamBuf::SocketStreamBuf(int sock_fd) : socketFd(sock_fd) {}

SocketStreamBuf::int_type	SocketStreamBuf::underflow( void )
{
	if (gptr() == egptr()) {
		// Buffer is empty, read more data from socket
		std::cout << "---------------recv() fd_i : " << socketFd << std::endl;
		int bytesRead = recv(socketFd, buffer, bufferSize, 0);
		if (bytesRead <= 0) {
			throw (SockEof());
		}
		setg(buffer, buffer, buffer + bytesRead);
	}
	return traits_type::to_int_type(*gptr());
}

const char*	SocketStreamBuf::SockEof::what( void ) const throw() {
	return ("Client Left or Sock Err");
}
