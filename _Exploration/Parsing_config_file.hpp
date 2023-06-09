#ifndef PARSING_CONFIG_FILE_HPP
#define PARSING_CONFIG_FILE_HPP

#include "include/Colors.hpp"
#include "Exceptions.hpp"

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
typedef struct s_conf_root_block : public t_conf_block {
	t_http_block	http;
}	t_conf_enclosing_block;

// assumptions
    // configuration file start at the server bloc level (no htpp or higher level blocs)

// throw ConfigFileException("");
    // check for bloc validity
        // wrong implementation of scope
        // server block missing
        // syntax error {} ; space
    // check for directive validity
        // directive missing, mandatory default values
        // directive unknown, syntax error
        // directive out of scope
        // directive duplicate
        // directive conflict
    // check for value validity
        // value invalid, syntax error
        // value invalid, file not found
        // value invalid, file unauthorized access

// mandatory directives
    // "listen", "location", "server_name", "index", "body_size", "error_page", 
    // "method", "root", "autoindex", "exec_cgi", "extension_cgi"

// blocks
    // http, server, location
// removes comment all characters after a # and up until a \n

void    comment_clean(std::string* config_file){
    for (
        size_t str_i = 0; 
        (str_i = config_file->find("#", str_i)) != std::string::npos;
        ){
            std::cout << config_file->substr(0, str_i) << RED << config_file->substr(str_i,  config_file.find("\n", str_i)) << RESET << config_file->substr( config_file.find("\n", str_i), npos) << std:endl;
            config_file->erase(str_i, config_file.find("\n", str_i));
    }
}

int    find_end_of_block(std::string& input, int begin){ // input and begin validity are checked beforehand
    if (DEBUG) std::cout << YELLOW << "find_end_of_block()" << RESET << std::endl;

    int     bracket_flag    = 0;
    int     i               = input.find("{", begin);

    if (i == std::string::npos || i < begin)
        throw std::invalid_argument("error find_end_of_block()");
    else
        bracket_flag++;
    
    for ( ; ( i < input.size() || bracket_flag == 0 ); i++ ){
        if (input.at(i) == "{")
            bracket_flag++;
        if (input.at(i) == "}")
            bracket_flag--;
    }
    return i;
}

std::pair<int, int> find_range_new_block(std::string& input, int begin_current, std::string next_block){
    if (DEBUG) std::cout << YELLOW << "find_range_new_block()" << RESET << std::endl;

    int begin_new = input.find(next_block, begin_current);

    if ( begin_new == std::string::npos )
        return; //! if no more block => which behavior
    int end_new = find_end_of_block(input, begin_new);

    return (std::make_pair(begin_new, end_new));    
}

template < BlockType = s_conf_root_block >
void    parse_new_block(std::string& input, int begin_current, int end_current, std::string next_block){
    if (DEBUG) std::cout << YELLOW << "parse_new_block()" << RESET << std::endl;

    std::pair<int, int> range = find_range_new_block(input, begin_current, next_block);    // handle no new block

    BlockType new_block;
    new_block.

    parse_directives(input, begin_current, begin_new);
    if (begin_new != std::string::npos )
        parse_new_block(input, begin_new, end_new, next_block);
    parse_directives(input, end_new, end_current);
}


void    parse_configuration_file(std::string config_file){
    comment_clean(&config_file);

    // parse root
    i = config_file.find("http{", 0) // find first http bloc
    if (i != std::string::npos){
        std::pair<int, int> range = find_range_new_block(config_file, i, "http");    // handle no new block
        parse_new_block<s_conf_root_block>(config_file, range.first, range.second, "server");

    }
}
   




    // std::istringstream input;
    // input.str(config_file);
    // s_conf_root_block root;                     // main level
    // root.directives = get_directives();
        // || config_file.find("server{", i) == std::string::npos \
    root.http = new_http_block();

    // int i = 0;
    // i = input.find("http{", i) // find first http bloc
    // if (i == std::string::npos)
    //     return; // create empty http and find first server
    // else if (i > 0){
    //     if (input.rfind("server{", i) != std::string::npos\
    //     || input.rfind("location{", i) != std::string::npos\        
    //     ){
    //         return; // no other bloc before http else throw
    //     }
    // }
    // else { // parse directives // begin is beg, it is j, end is end
    //     std::map<std::string, std::string>	dir;
    //     int beg = input.find_first_not_of("   ", 0);
    //     if (beg > i)
    //         return;
    //     int end = i;
    //     int it = beg;
    //     it = input.find("\n", beg) // find first \n
    //     if (it == std::string::npos && no white space)
    //         return; throw invalid syntax, directive missing ;

    // }
    //     for (
    //     std::string::iterator str_it = config_file.begin(); 
    //     str_it = config_file->find(str_it, config_file.end(), "server") != config_file.end();
    //     ;
    //     ){
    //         str_it = config_file->erase(str_it, config_file.find(str_it, config_file.end(), "\n"));
    // }


#endif 

