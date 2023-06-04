/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:49:33 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/04 15:10:45 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketStream.hpp"

SocketStreamBuf::SocketStreamBuf(int sock_fd) :
	socketFd(sock_fd)
{}

//!	vulnerabilità (si verifica di rado con buffer_size moderatamente grande, spesso con piccola)
//!
//!	getline legge dallo stream fin quando una delle 2 condizioni che seguono non si avvera
//!		1. il delimiter è letto
//!		2. non ci sono più caratteri nello stream (eof)
//!
//!	La seconda implica che getline non si ferma se non ci sono più caratteri nel buffer
//!	dello streambuf, ma se non ci sono più caratteri nel source.
//!	underflow() viene chiamata più volte fin quando una delle 2 condizioni di sopra
//!	non si avvera.
//!
//!	problema:	chiamo recv() più volte avendo la garanzia di poterlo fare (mediante epoll())
//!				solo la prima volta!
//!				Quindi, dovrei portarmi dietro epoll() e nel qual caso lanciare una eccezione specifica (NOT_AVAIL)
//!				(P.s: non posso tornare type_traits::eof() nè *gptr()
//!				perchè il primo setta eof() nello stream, l'altro non è deferenziabile;)
//!				tornare qualsiasi altro carattere non è safe).
//!
//!		problema 2 :	non posso lanciare eccezioni in underflow perchè vengono catturate da getline o
//!						altre funzioni di estrazione; ergo non ho controllo al di fuori.
//!
//!
SocketStreamBuf::int_type	SocketStreamBuf::underflow( void )
{//std::cout  << std::endl << "\033[1m\033[32m""underflow() called()""\033[0m" << std::endl;

	if (gptr() == egptr()) {
		
		//* Buffer is empty, read more data from socket
		std::cout << "---------------recv() fd_i : " << socketFd << std::endl;
		int bytesRead = recv(socketFd, buffer, bufferSize, 0);
		if (bytesRead <= 0) {
			return traits_type::eof();
		}
		setg(buffer, buffer, buffer + bytesRead);
	}
	// std::cout << std::endl;
	return traits_type::to_int_type(*gptr());
}

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
