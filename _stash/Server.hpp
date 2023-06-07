/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:35:54 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/07 11:56:23 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

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

class Server{
private:
	//*		Member variables
		int					server_fd;
		int					server_port;
		struct sockaddr_in	server_addr;
		socklen_t			server_addr_len;

public:
	Server();
	~Server();

private:
	//*		private initialization functions
	void _server_init();
	void _create_server_socket();
	void _set_socket_as_reusable();
	void _init_server_addr();
	void _print_server_ip_info();
	void _bind_server_socket_to_ip();
	void _make_server_listening();
	void _make_socket_non_blocking(int sock_fd);

	//*		Canonical Form
	Server(const Server & cpy);
	Server& operator=(const Server & cpy);
};

#endif