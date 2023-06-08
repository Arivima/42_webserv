/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config_types.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 11:02:02 by earendil          #+#    #+#             */
/*   Updated: 2023/06/08 20:06:44 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <vector>
#include <map>

typedef struct s_conf_block {

	std::map<std::string, std::string>	directives;

}	t_conf_block;

typedef struct s_location_block : public t_conf_block {

	s_location_block(const std::map<std::string, std::string>& dir) {
		this->directives = dir;
	}
}	t_location_block;

typedef struct s_server_block : public t_conf_block {

	std::vector<t_location_block> locations;

	s_server_block(const std::map<std::string, std::string>& dir) : locations() {
		this->directives = dir;
	}
}	t_server_block;


typedef struct s_http_block : public t_conf_block {

	std::vector<struct s_server_block>	servers;

	s_http_block(const std::map<std::string, std::string>& dir) : servers() { 
		this->directives = dir; 
	}

}	t_http_block;

typedef struct s_conf_enclosing_block : public t_conf_block {
	
	t_http_block	http;
	
}	t_conf_enclosing_block;
























//*		OLD
// struct s_http_block;
// struct s_server_block;
// struct s_location_block;

// typedef struct s_block {
// 	std::map<std::string, std::string>	directives;
// }	t_block;

// typedef struct s_http_block : public t_block {

// 	std::vector<struct s_server_block *> servers;

// 	s_http_block(const std::map<std::string, std::string>& dir) : servers() { 
// 		this->directives = dir; 
// 	}
// 	~s_http_block(void) {
// 		for (
// 			std::vector<struct s_server_block *>::iterator it = servers.begin();
// 			it != servers.end();
// 		)
// 			it = servers.erase(it);
// 	}

// }	t_http_block;

// typedef struct s_server_block : public t_block {

// 	std::vector<struct s_location_block *> locations;

// 	s_server_block(const std::map<std::string, std::string>& dir) : locations() {
// 		this->directives = dir;
// 	}
// 	~s_server_block(void) {
// 		for (
// 			std::vector<struct s_location_block *>::iterator it = locations.begin();
// 			it != locations.end();
// 		)
// 			it = locations.erase(it);
// 	}
// }	t_server_block;

// typedef struct s_location_block : public t_block {

// 	s_location_block(const std::map<std::string, std::string>& dir) {
// 		this->directives = dir;
// 	}
// 	~s_location_block(void);
// }	t_location_block;
