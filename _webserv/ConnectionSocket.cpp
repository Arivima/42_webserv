/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:07:39 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/10 16:03:31 by avilla-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/ConnectionSocket.hpp"
#include <iostream>		//cout
#include <unistd.h>

//*		Main Constructor
ConnectionSocket::ConnectionSocket(
								int							sock_fd,
								const std::string&			client_IP,
								const std::string&			server_IP,
								const t_server&				assigned_server,
								const t_epoll_data&			edata
) :
	sock_fd(sock_fd),
	client_IP(client_IP),
	server_IP(server_IP),
	assigned_server(assigned_server),
	edata(edata)
{
	status = e_REQ_MODE;
	request = new Request(sock_fd, edata);
	response = NULL;
}


//*		Main Functions

void	ConnectionSocket::status_switch( void ) {
	if (e_REQ_MODE == this->status) {
		COUT_DEBUG_INSERTION(YELLOW "ConnectionSocket::status_switch()---response" RESET << std::endl);
		request->print_req();
		response = new Response(
			request,
			assigned_server,
			sock_fd,
			client_IP,
			server_IP,
			edata
		);
		response->generateResponse();
		response->print_resp();
		status = e_RESP_MODE;
	}
	else {
		COUT_DEBUG_INSERTION(YELLOW "ConnectionSocket::status_switch()---request" RESET << std::endl);
		delete response;
		response = NULL;
		delete request;
		request = NULL;
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
			if (response->dechunking())
				response->POSTNextChunk();
			else
				response->send_line();
		}
	}
	catch (const TaskFulfilled& e) {
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
