/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:43:27 by earendil          #+#    #+#             */
/*   Updated: 2023/06/09 10:43:01 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_HPP
# define TYPES_HPP

#include "include/webserv.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

class ConnectionSocket;
class t_server_block;

typedef std::vector<ConnectionSocket* >		VectorCli;

typedef struct s_server {
	const t_server_block&		conf_server_block;
	int							server_fd;
	int							server_port;
	struct sockaddr_in			server_addr;
	socklen_t					server_addr_len;

	VectorCli					requests;
	
	s_server(const t_server_block& server_block) : conf_server_block(server_block) {}
}	t_server;

typedef std::vector<t_server>				VectorServ;

#endif