/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:35:54 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/07 12:57:14 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Worker_HPP
# define Worker_HPP

// INCLUDES
#include <iostream>         // cin cout cerr
#include <string>           // strings, c_str()
#include <cstring>          // memset

#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv, bind, socklen_t
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_ntop
#include <fcntl.h>          // fcntl
#include <limits.h>			// MAX INT

#include <sys/select.h>     // select, FT_ISSET, FT_CLR, etc

#include <map>
#include <vector>           // vector

#include <unistd.h>         // close
#include <stdlib.h>         // exit, EXIT_FAILURE

#include "Webserv.hpp"
#include "ConnectionSocket.hpp"
#include "EpollData.hpp"
// #include "Exceptions.hpp"

class Worker{

	//*		TYPEDEFS
public:

	//*		Member variables
private:
	VectorServ				servers;
	t_epoll_data			edata;

	//*		Member functions
public:
	Worker(const t_conf_block& conf_enclosing_block);
	~Worker();
	
	void workerLoop();

private:
	//*		private helper functions
	void	_io_multiplexing_using_epoll();
	void	_serve_clientS( void );
	// void	_serve_client( ConnectionSocket& request );
	void	_handle_new_connectionS( void );
	void	_handle_new_connection();

	//*		private initialization functions
	void	_server_init(const t_conf_block& conf_server_block);
	void	_create_server_socket();
	int		_create_ConnectionSocket(
		t_server& server,
		std::string& client_IP, std::string& server_IP
	);
	void	_epoll_register_ConnectionSocket(int cli_socket);
	void	_set_socket_as_reusable();
	void	_init_server_addr(
		const std::map<std::string, std::string>& server_directives
	);
	void	_print_server_ip_info();
	void	_bind_server_socket_to_ip();
	void	_make_server_listening();
	void	_make_socket_non_blocking(int sock_fd);
	void	_init_io_multiplexing();

	//*		Canonical Form
	Worker(const Worker & cpy);
	Worker& operator=(const Worker & cpy);

};

#endif