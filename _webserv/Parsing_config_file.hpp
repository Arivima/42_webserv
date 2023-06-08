#ifndef PARSING_CONFIG_FILE_HPP
#define PARSING_CONFIG_FILE_HPP

#include "include/Colors.hpp"
#include "Exceptions.hpp"

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


void    comment_clean(std::string* config_file);

void    parse_configuration_file(char* s){
    // clean the string
    std::string config_file(s);
    comment_clean(&config_file);
    // parse by blocs
    //     for (
    //     std::string::iterator str_it = config_file.begin(); 
    //     str_it = config_file->find(str_it, config_file.end(), "server") != config_file.end();
    //     ;
    //     ){
    //         str_it = config_file->erase(str_it, config_file.find(str_it, config_file.end(), "\n"));
    // }




}



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

#endif 

