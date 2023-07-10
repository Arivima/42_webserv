/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 17:56:31 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/10 20:38:08 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONSOCKET_HPP
# define CONNECTIONSOCKET_HPP

#include <sys/socket.h>	// socket, AF_INET, SOCK_STREAM, recv, bind, socklen_t
#include <arpa/inet.h>	//	sockaddr_in

# include "Webserv.hpp"
# include "Response.hpp"
# include "Request.hpp"

/**
 * @brief This class represents an open connection with a client.
 * It is responsible for the handling of the whole
 * of the connection life cycle.
 */
class ConnectionSocket
{
//*		Public typedefs ____________________________________________
public:
	typedef enum e_CONNECTION_STATUS
	{
		e_REQ_MODE,
		e_RESP_MODE,
	}	t_CONNECTION_STATUS;

//*		Private member attributes ___________________________________
private:
	t_CONNECTION_STATUS						status;						//*	REQUEST or RESPONSE
	const int								sock_fd;					//*	connection socket fd
	const std::string						client_IP;					//*	ip of remote client
	const std::string						server_IP;					//*	the interface, among all the assigned_server utilized interfaces, where the connection got accepted.
	const t_server&							assigned_server;			//* struct containing server + open connections + server conf block
	const t_epoll_data&						edata;						//*	epoll data reference
	Response*								response;					//*	response data of current request
	Request*								request;					//*	request data of current request

//*		Public member functions _____________________________________
public:
	//*		main Constructors and Destructors
						ConnectionSocket(
								int							sock_fd,
								const std::string&			client_IP,
								const std::string&			server_IP,
								const t_server&				assigned_server,
								const t_epoll_data&			edata
						);
						~ConnectionSocket();
	//*	main functionalities
	void				serve_client( void );
	int					getSockFD( void );
	t_CONNECTION_STATUS	getStatus( void );
	const t_server&		getAssignedServer( void ) const;

//*		Private member functions ____________________________________
private:
	//* Unused canonical form
						ConnectionSocket( void );
						ConnectionSocket( const ConnectionSocket & sock );
	ConnectionSocket&	operator=(const ConnectionSocket& sock);
	//*		helper functions
	void				status_switch( void );
};
#endif