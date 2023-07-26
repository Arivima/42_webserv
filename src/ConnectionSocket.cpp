/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.cpp                               :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionSocket.hpp"
# include	"Utils.hpp"

#include <iostream>	
#include <unistd.h>

//*		Main Constructor
ConnectionSocket::ConnectionSocket(
								int							sock_fd,
								const std::string&			client_IP,
								const std::string&			server_IP,
								const t_server&				assigned_server,
								const t_epoll_data&			edata,
								long long&					cur_memory_usage
) :
	sock_fd(sock_fd),
	client_IP(client_IP),
	server_IP(server_IP),
	assigned_server(assigned_server),
	edata(edata),
	cur_memory_usage(cur_memory_usage)
{
	status = e_REQ_MODE;
	request = new Request(sock_fd, edata, cur_memory_usage);
	response = NULL;
}


//*		Main Functions

void	ConnectionSocket::status_switch( void )
{
	bool	close_connection;

	if (e_REQ_MODE == this->status) {
		COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW "ConnectionSocket::status_switch()---response" RESET << std::endl);
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
		if (false == response->isDechunking())
			response->print_resp();
		status = e_RESP_MODE;
	}
	else {
		COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW "ConnectionSocket::status_switch()---request" RESET << std::endl);
		close_connection = (request->timedOut() || response->isRedirect());
		delete request;
		request = NULL;
		delete response;
		response = NULL;
		if (close_connection)
			throw (SockEof());
		status = e_REQ_MODE;
	}
}

void	ConnectionSocket::serve_client( void )
{
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);

	try {
		if (e_REQ_MODE == status)
		{
			if (NULL == request)
			{
				if (NULL != eevent &&  eevent->events & EPOLLIN)
					request = new Request(sock_fd, edata, cur_memory_usage);
				else
					return ;
			}
			request->parse_line();
		}
		else
		{
			if (response->isDechunking())
				response->handle_next_chunk();
			else
				response->send_line();
		}
	}
	catch (const TaskFulfilled& e) {
		this->status_switch();
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
