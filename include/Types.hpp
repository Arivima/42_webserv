/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Types.hpp                                          :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_HPP
# define TYPES_HPP

# include <string>
# include <sstream>
# include <iostream>		//cout
# include <fstream>			//ifstream
# include <sys/socket.h>
# include <netinet/in.h>
# include <vector>			//config file data type sub-block and error_page in HttpError exception.
# include <map>

//*			Parsing
	typedef enum e_config_block_level {
		e_root_block = 0,
		e_http_block,
		e_server_block,
		e_virtual_server_block,
		e_location_block,
	}	t_config_block_level;


	typedef struct s_conf_block {
		t_config_block_level				level;
		std::map<std::string, std::string>	directives;
		std::vector<struct s_conf_block>	sub_blocks;
		bool								invalidated;

		s_conf_block(
			t_config_block_level lvl = e_root_block,
			std::map<std::string, std::string> dir = std::map<std::string, std::string>()
		);
	}	t_conf_block;


	class ConnectionSocket;

	typedef std::vector<ConnectionSocket* >		VectorCli;


	typedef struct s_server {
		const t_conf_block&			conf_server_block;
		int							server_fd;
		int							server_port;
		struct sockaddr_in			server_addr;
		socklen_t					server_addr_len;

		VectorCli					open_connections;
		
		s_server(const t_conf_block& server_block) : conf_server_block(server_block) {}
	}	t_server;

	typedef std::vector<t_server>				VectorServ;


//*		type utilities -> utils.cpp
	t_config_block_level	next_conf_block_level(t_config_block_level lvl);
	int						block_get_level(std::string block_name);
	std::string 			block_get_name(t_config_block_level level);
	bool					mandatory_server_directives_present(const t_conf_block& current);
	bool					same_server(
		const t_conf_block& server,
		const t_conf_block& virtual_serv2
		);
	bool					same_host(
		const t_conf_block& virtual_serv1,
		const t_conf_block& virtual_serv2
	);
	void					print_block(t_conf_block& block, size_t level);
	void					print_directives(
		std::map<std::string, std::string>& directives,
		size_t level
		);
	std::ostream&			operator<<(
		std::ostream& stream, const t_config_block_level& block
		);

#endif