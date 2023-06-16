/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config_types.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 11:02:02 by earendil          #+#    #+#             */
/*   Updated: 2023/06/16 14:23:52 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_TYPES_HPP
# define CONFIG_TYPES_HPP

# include <vector>
# include <map>
# include <ostream>

# define BLOCK_NAME_ROOT        "root"
# define BLOCK_NAME_HTTP        "http"
# define BLOCK_NAME_SERVER      "server"
# define BLOCK_NAME_LOCATION    "location"

// enum
typedef enum e_config_block_level {
	e_root_block = 0,
	e_http_block,
	e_server_block,
	e_location_block,
}	t_config_block_level;

std::ostream&	operator<<(std::ostream& stream, const t_config_block_level& block) {

	stream << type_get_name(block.block_name);

	return (stream);
}

//Prototypes
std::string type_get_name(t_config_block_level level);

// struct
typedef struct s_conf_block {
    t_config_block_level				level;
    std::string							block_name;
	std::map<std::string, std::string>	directives;
    std::vector<struct s_conf_block>	sub_blocks;

	s_conf_block(
		t_config_block_level lvl = e_root_block,
		std::map<std::string, std::string> dir = std::map<std::string, std::string>()
	) 
    : level(lvl), block_name(type_get_name(lvl)), sub_blocks(), directives(dir)
	{
        if (DEBUG) std::cout << "Build a new : " << this->level << std::endl;
    }
}	t_conf_block;

std::string type_get_name(t_config_block_level level){
	switch(level) {
		case e_root_block: return BLOCK_NAME_ROOT;
		case e_http_block: return BLOCK_NAME_HTTP;
		case e_server_block: return BLOCK_NAME_SERVER;
		case e_location_block: return BLOCK_NAME_LOCATION;
	}
}


/// printing functions
/*brief*/   // prints contents of a map of string directives
void    print_directives(std::map<std::string, std::string>& directives){
    std::cout << "| Directives :" << std::endl;
    for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); it++)
        std::cout << "|  Key/Value |" << (*it).first << "|" << (*it).second << "|" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
}
/*brief*/   // recursive call
void    print_block(t_conf_block& block, size_t level){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing print_level #" << block.level << " | " << block.block_name << std::endl;
    std::cout << "| Printing block_level #" << level << " |" << std::endl;
    print_directives(block.directives);
    for (std::vector<t_conf_block>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        print_block(*it, level + 1);
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////







// typedef struct s_conf_block {

// 	std::map<std::string, std::string>	directives;

// }	t_conf_block;

// typedef struct s_location_block : public t_conf_block {

// 	s_location_block(const std::map<std::string, std::string>& dir) {
// 		this->directives = dir;
// 	}
// }	t_location_block;

// typedef struct s_virtual_server_block : public t_conf_block {

// 	std::vector<t_location_block> locations;

// 	s_virtual_server_block(const std::map<std::string, std::string>& dir) : locations() {
// 		this->directives = dir;
// 	}
// }	t_virtual_server_block;

// typedef struct s_server_block : public t_conf_block {

// 	std::vector<t_virtual_server_block> virtual_servers;

// 	s_server_block(const std::map<std::string, std::string>& dir) : virtual_servers() {
// 		this->directives = dir;
// 	}
// }	t_server_block;

// typedef struct s_http_block : public t_conf_block {

// 	std::vector<struct s_server_block>	servers;

// 	s_http_block(const std::map<std::string, std::string>& dir) : servers() { 
// 		this->directives = dir; 
// 	}

// }	t_http_block;

// typedef struct s_conf_root_block : public t_conf_block {
	
// 	t_http_block	http;
	
// }	t_conf_root_block;


#endif





















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
