/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:49:33 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/29 18:52:55 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketStream.hpp"

explicit SocketStreamBuf::SocketStreamBuf(int sock_fd) : socketFd(socketFd) {}

SocketStreamBuf::int_type	SocketStreamBuf::underflow( void )
{
	if (gptr() == egptr()) {
		// Buffer is empty, read more data from socket
		int bytesRead = recv(socketFd, buffer, bufferSize, 0);
		if (bytesRead <= 0) {
			// Error or end of stream
			return traits_type::eof();
		}
		setg(buffer, buffer, buffer + bytesRead);
	}
	return traits_type::to_int_type(*gptr());
}