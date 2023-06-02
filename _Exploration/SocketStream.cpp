/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:49:33 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/02 12:10:04 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketStream.hpp"

SocketStreamBuf::SocketStreamBuf(int sock_fd) :
	socketFd(sock_fd)
{}

SocketStreamBuf::int_type	SocketStreamBuf::underflow( void )
{
	if (gptr() == egptr()) {
		
		//* Buffer is empty, read more data from socket
		std::cout << "---------------recv() fd_i : " << socketFd << std::endl;
		int bytesRead = recv(socketFd, buffer, bufferSize, 0);
		if (bytesRead <= 0) {
			return traits_type::eof();
		}
		setg(buffer, buffer, buffer + bytesRead);
	}
	return traits_type::to_int_type(*gptr());
}

// std::streamsize	SocketStreamBuf::xsgetn(char* s, std::streamsize n) {
// 	if (gptr() == egptr())
// 		return (0);
	
// 	const std::streamsize	availble = egptr() - gptr();
// 	const std::streamsize	to_read = std::min(availble, n);

// 	std::copy(gptr(), gptr() + to_read, s);
// 	gbump(static_cast<int>(to_read));

// 	if (to_read < n && underflow() != traits_type::eof())
// 		return (to_read + xsgetn(s + to_read, n - to_read));
// 	return (to_read);
// }

bool	SocketStreamBuf::has_eol( void )
{
	char*	ptr = gptr();
	char*	end = egptr();

	while (ptr != end) {
		if ('\n' == *ptr)
			return (true);
		ptr ++;
	}
	return (false);
}
