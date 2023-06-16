/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:26:23 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/16 18:14:20 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

//*		non member helper functions Prototypes
static std::string    get_whole_line(std::string& line, size_t i);
std::pair<bool, std::string> get_unknown_block_name(std::string& line, size_t i);
//*		/////////////////////////////

Config::Config( const char* config_file_path ) : conf() {
	
	read_config_file(
		NULL == config_file_path ? DEFAULT_PATHNAME : config_file_path
	);

}

/*brief*/   // parse configuration file into configuration structures, checks validity of input
// URGENT   // fix several server
//TODO      // add virtual server
            // implement max and min number of blocks 
//TODO      // add all validity checks
//TODO      // redo a flow run to catch all loose error/exception/cases threads
//TODO      // thorough testing
void	Config::parse( void ) { if (DEBUG) std::cout << YELLOW << "parse_configuration_file()" << RESET << std::endl;
	
	size_t						start	= 0;
	size_t						end		= this->raw_content.size() - 1;
	
	std::pair<int, std::string> level = get_unknown_block_level(config_file, 0);
	if (level.first == -1)
	    throw std::invalid_argument("error configuration file : " + level.second);
	
	t_conf_block    root_block(level.first); // name in level.second si besoin
	parse_block(config_file, std::make_pair( start, end ), root_block);
	
	print_config_blocks(root_block);
}





//*		private helper functions
void	Config::read_config_file(std::string conf_pathname){if (DEBUG) std::cout << YELLOW << "read_config_file()" << RESET << std::endl;

	const size_t	buffer_size = 4096;
	char			buf[buffer_size];
	size_t			bytes_read;
	
	int fd = open(conf_pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		throw std::runtime_error("error open"); //!
	}
	memset(buf, '/0', buffer_size);
	while ((bytes_read = read(fd, buf, buffer_size)) > 0) {
		this->raw_content.append(buf);
		memset(buf, '/0', buffer_size);
	}
	if (-1 == bytes_read) {
		if (-1 == close(fd))
			throw std::runtime_error("error read && close"); //!
		throw std::runtime_error("error read"); //!
	}
	else if (0 == bytes_read) {
		if (DEBUG) std::cout << MAGENTA << "EOF Finished reading configuration file" << RESET << std::endl;
	}
	remove_comments();
}

/*brief*/   // removes comment all comments = characters after a # and up until a \n
void	Config::remove_comments( void ){if (DEBUG) std::cout << YELLOW << "remove_comments()" << RESET << std::endl;
	
	size_t	nl_pos;
	
	for (size_t i = 0; (i = this->raw_content.find("#", i)) != std::string::npos;) {
		if (DEBUG)
			std::cout << "Found string to delete at pos : " << i << std::endl
			<<  RED << get_whole_line(this->raw_content, i) << RESET << std::endl;
		
		nl_pos = this->raw_content.find("\n", i);
		if (std::string::npos == nl_pos)
			this->raw_content.erase(i);
		else
			this->raw_content.erase(i, nl_pos - i + 1);
	}
}


//*		non member helper functions

/*brief*/   // extracts and returns the whole line where i is positioned
static std::string    get_whole_line(std::string& line, size_t i) {
	size_t start, len;
	start = line.rfind("\n", i);
	if (start != std::string::npos)
		start++;
	start = (start != std::string::npos ? start : i);
	len = line.find("\n", i);
	len = (len != std::string::npos ? len : i) - start + 1;
	return (line.substr(start, len));
}

/*brief*/   // find any next block, and identifies the name 
            // location blocks : returns the whole line not just name
std::pair<bool, std::string> get_unknown_block_name(std::string& line, size_t i) {
	size_t start, block_start, len;
	
	block_start = line.find("{", i);
	if (block_start == std::string::npos)
	    return (std::make_pair(EXIT_FAILURE, "get_unknown_block_name : no configuration block identified"));
	
	start = line.rfind("\n", block_start);
	if (start == std::string::npos)
	    start = 0;
	else
	    start++;
	len = block_start - start + 1;
	return (std::make_pair(EXIT_SUCCESS, line.substr(start, len)));
}
