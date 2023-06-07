/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:35:54 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/07 13:57:04 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKERSERVER_HPP
#define WORKERSERVER_HPP

// INCLUDES
#include <iostream>         // cin cout cerr
#include <string>           // strings, c_str()
#include <cstring>          // memset

#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv, bind, socklen_t
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_ntop
#include <fcntl.h>          // fcntl

#include <sys/select.h>     // select, FT_ISSET, FT_CLR, etc

#include <vector>           // vector

#include <unistd.h>         // close
#include <stdlib.h>         // exit, EXIT_FAILURE

#include "../include/webserv.hpp"
#include "ConnectionSocket.hpp"
#include "EpollData.hpp"
#include "Exceptions.hpp"

class WorkerServer{

	//*		TYPEDEFS
private:

	typedef  std::vector<ConnectionSocket *>	VectorCli;

	//*		Member variables
private:
	// std::vector<t_server>			servers;
	t_epoll_data					edata;
	std::vector<ConnectionSocket *>	clients;

public:
	WorkerServer();
	~WorkerServer();

	void serverLoop();

private:
	//*		private helper functions
	
	void _io_multiplexing_using_epoll();
	void _handle_new_connection();
	void	_serve_client( ConnectionSocket& client );

	//*		private initialization functions
	
	void _init_io_multiplexing();

	//*		Canonical Form
	WorkerServer(const WorkerServer & cpy);
	WorkerServer& operator=(const WorkerServer & cpy);

};


#endif