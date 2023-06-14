/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/14 18:36:15 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
#include <iostream>	//cout
#include <unistd.h>

//*		Main Constructor
ConnectionSocket::ConnectionSocket(
								int sock_fd,
								const t_server& assigned_server,
								const t_epoll_data& edata) :
	sock_fd(sock_fd),
	assigned_server(assigned_server),
	edata(edata)
{
	status = e_REQ_MODE;
	request = new Request(sock_fd, edata);
	response = NULL;
}


//*		Main Functions

void	ConnectionSocket::status_switch( void ) {
	if (e_REQ_MODE) {
		if (response)
			delete response;
		response = new Response(
			request->getRequest(),
			assigned_server,
			sock_fd,
			edata
		);
		status = e_RESP_MODE;
	}
	else {
		if (request)
			delete request;
		request = new Request(sock_fd, edata);
		status = e_REQ_MODE;
	}
}

void	ConnectionSocket::serve_client( void ) {
	try {
		if (e_REQ_MODE == status) {
			request->parse_line();
		}
		else {
			response->send_line();
		}
	}
	catch (const TaskFulfilled& e) {
		std::cout << e.what() << std::endl;
		this->status_switch();
		this->serve_client();
	}
}

//*		Utilities

int	ConnectionSocket::getSockFD( void ) {
	return (this->sock_fd);
}

const t_server&	ConnectionSocket::getAssignedServer( void ) const
{
	return (this->assigned_server);
}

ConnectionSocket::t_CONNECTION_STATUS	ConnectionSocket::getStatus( void ) {
	return (this->status);
}

//*		CANONICAL FORM

ConnectionSocket::~ConnectionSocket( void )
{
	if (request)
		delete request;
	if (response)
		delete response;
	close(sock_fd);
}
