#include "../_webserv/include/Colors.hpp"
#include "../_webserv/include/Types.hpp"
#include <set>
#include <string>
#include <iostream>
# include <stdexcept>

void	Config::check_directive_validity(const std::string& directive, t_config_block_level level){
	std::cout << " check_directive_validity | directive : " << directive << " | level : " << level << std::endl;

	static const char*					allowed_directives[] = {
		"listen", "location", "server_name", "host", "index", "body_size",
		"error_page", "method", "root", "return", "autoindex",
		"exec_cgi", "extension_cgi"
	};
	static const char*					virtual_server_directives[] = {
		"listen", "location", "server_name", "host", "index",
		"body_size", "error_page", "method", "root", "return",
		"autoindex", "exec_cgi", "extension_cgi"
	};
	static const char*					location_directives[] = {
		"location", "index", "body_size", "error_page", "method", "root",
		"return", "autoindex", "exec_cgi", "extension_cgi"
	};
	static const std::set<std::string>	dictionnary_all_directives(allowed_directives, allowed_directives + sizeof(allowed_directives) / sizeof(allowed_directives[0]));
	static std::set<std::string>		dictionnary_block_directives;
	size_t								dict_size;

	if (dictionnary_all_directives.count(directive) == 0)
		throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " is an unrecognized directive"));

	const char** block_directives;
	switch(level) {
		case e_virtual_server_block:
			block_directives = virtual_server_directives;
			dict_size = sizeof(virtual_server_directives) / sizeof(virtual_server_directives[0]);
			break;
        case e_location_block: 
			block_directives = location_directives;
			dict_size = sizeof(location_directives) / sizeof(location_directives[0]);
			break;
		default:
			throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " directive declared on the wrong scope"));
    }

	dictionnary_block_directives = std::set<std::string>(block_directives, block_directives + dict_size);
	if (dictionnary_block_directives.count(directive) == 0)
		throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " directive declared on the wrong scope"));
}

int main (){


    // check_directive_validity(*it, i)
        return 0;
}