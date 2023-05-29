/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/29 19:01:13 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
#include "SocketStream.hpp"

ConnectionSocket::ConnectionSocket(int sock_fd) :
	stream_buf(SocketStreamBuf(sock_fd)),
	stream(std::istream(&this->stream_buf))
{
	this->sock_fd = sock_fd;
}

ConnectionSocket::t_PARSE_RET	ConnectionSocket::parse_line( void )
{
}
