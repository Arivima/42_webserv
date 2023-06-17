/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:26:23 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/17 23:05:03 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <algorithm>	//remove_if
#include <cstring>		//strncmp

//*		non member helper functions Prototypes
static std::string				get_whole_line(std::string& line, size_t i);
std::pair<bool, std::string>	get_unknown_block_name(std::string& line, size_t i);
std::pair<int, std::string>		get_unknown_block_level(std::string& line, size_t i);
int								type_get_level(std::string block_name);
//*		/////////////////////////////

t_conf_block::s_conf_block(
		t_config_block_level lvl = e_root_block,
		std::map<std::string, std::string> dir = std::map<std::string, std::string>()
	)
	: level(lvl), block_name(type_get_name(lvl)), directives(dir), sub_blocks()
{
	if (DEBUG) std::cout << "Build a new : " << this->level << std::endl;
}

Config::Config( const char* config_file_path ) : conf() {
	
	read_config_file(
		NULL == config_file_path ? DEFAULT_PATHNAME : config_file_path
	);
	content_stream.str(this->raw_content);
	std::cout << "conf file: " << this->raw_content << std::endl;
}

Config::~Config( void ) {
}

// /*brief*/   // parse configuration file into configuration structures, checks validity of input
// // URGENT   // fix several server
// //TODO      // add virtual server
//             // implement max and min number of blocks 
// //TODO      // add all validity checks
// //TODO      // redo a flow run to catch all loose error/exception/cases threads
// //TODO      // thorough testing
// void	Config::parse( void ) { if (DEBUG) std::cout << YELLOW << "parse_configuration_file()" << RESET << std::endl;
	
// 	size_t						start	= 0;
// 	size_t						end		= this->raw_content.size() - 1;
	
// 	std::pair<int, std::string> level = get_unknown_block_level(this->raw_content, 0);
// 	if (level.first == -1)
// 		 throw std::invalid_argument("error configuration file : " + level.second);
	
// 	t_conf_block	root_block(static_cast<t_config_block_level>(level.first)); // name in level.second si besoin
// 	parse_block(config_file, std::make_pair( start, end ), root_block);
	
// 	print_config_blocks(root_block);
// }

//*		private helper functions
void	Config::read_config_file(std::string conf_pathname){if (DEBUG) std::cout << YELLOW << "read_config_file()" << RESET << std::endl;

	const size_t	buffer_size = 4096;
	char			buf[buffer_size + 1];
	int				bytes_read;
	
	int fd = open(conf_pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		throw std::runtime_error("error open"); //!
	}
	memset(buf, '\0', buffer_size + 1);
	while ((bytes_read = read(fd, buf, buffer_size)) > 0) {
		this->raw_content.append(buf);
		memset(buf, '\0', buffer_size + 1);
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

void	Config::parse_config( void ) {
	this->parse(this->conf);

	print_block(this->conf, this->conf.level);
}

struct IsSpacePredicate {
    bool operator()(char c) const {
        return std::isspace(static_cast<unsigned char>(c));
    }
};

std::string		strip_spaces(std::string& str) {
	std::string	stripped;

	std::cout << "strip spaces" << std::endl;
	stripped = str;
	stripped.erase(
		std::remove_if(stripped.begin(), stripped.end(), IsSpacePredicate()),
		stripped.end()
	);
	std::cout << "strip spaces------END" << std::endl;
	return (stripped);
}

void		strip_trailing_and_leading_spaces(std::string& str) {
	std::string::iterator			it;
	std::string::reverse_iterator	rit;

	//*		remove all trailing whitespaces up unitl first non-space char
	for (it = str.begin(); it != str.end(); ) {
		if (std::isspace(*it))
		{
			// std::cout << "GOTCHA!" << std::endl;
			it = str.erase(it);
		}
		else {
			// std::cout << "no space char: " << *it << std::endl;
			break ;
		}
	}
	//*		remove all leading whitespaces
	for (rit = str.rbegin(); rit != str.rend(); rit++) {
		if (std::isspace(*rit)) {
			it = (rit + 1).base();
			it = str.erase(it);
			rit = std::string::reverse_iterator(it);
		}
		else
			break;
	}
		// std::cout << "read line: |" << str << std::endl;

}

//WIP   // check find or compare
int	block_get_level(std::string block_name) {
	if ("root{" == strip_spaces(block_name))
		return e_root_block;
	if ("http{" == strip_spaces(block_name))
		return e_http_block;
	if ("server{" == strip_spaces(block_name))
		return e_server_block;
	if (0 == strncmp("location", block_name.c_str(), strlen("location")))
		return e_location_block;
	return -1;
}

t_config_block_level	next_conf_block_level(t_config_block_level lvl) {
	if (e_root_block == lvl)
		return (e_http_block);
	if (e_http_block == lvl)
		return (e_server_block);
	if (e_server_block== lvl || e_virtual_server_block == lvl)
		return (e_location_block);
	throw (
		std::runtime_error(
			"next_conf_block_level(): no level further than location")
		);
	return (e_root_block);//*	meaningless
}

void	Config::parse_http_block( t_conf_block& current ) {
	
	std::cout << "parse_http_block()" << std::endl;
	t_conf_block	http(
		e_http_block,
		current.directives
	);
	current.sub_blocks.push_back(http);
	parse(current.sub_blocks.back());
}

void	Config::parse_location_block( t_conf_block& current, std::string& cur_line  ) {
	
	std::cout << "parse_location_block()" << std::endl;
	std::stringstream	linestream(cur_line);
	std::string			keyword;
	std::string			path;

	std::getline(linestream, keyword, ' ');
	std::getline(linestream, path);
	t_conf_block		location(
		e_location_block,
		current.directives
	);
	location.directives[keyword] = path;
	current.sub_blocks.push_back(location);
	parse(current.sub_blocks.back());
}

void	Config::parse_server_block( t_conf_block& current ) {
	
	std::cout << "parse_server_block()" << std::endl;
	std::vector<t_conf_block>::iterator	it;
	// std::vector<t_conf_block>::iterator	it2;
	t_conf_block						virtual_server(
		e_virtual_server_block,
		current.directives
	);
	parse(virtual_server);//* current is http and sub_blocks server. Each server has list of virtual servers.
	for ( it = current.sub_blocks.begin(); it != current.sub_blocks.end(); it ++)
		if (
			// directives.end() != (*it).directives.find("port") &&
			// virtual_server.directives.end() != virtual_server.directives.find("port") &&
			//(*it).directives.at("port")
			(*it).sub_blocks[0].directives.at("listen") == virtual_server.directives.at("listen"))
		{
			// for (it2 = (*it).sub_blocks.begin(); it2 != (*it).sub_blocks.end(); it2++)
			// 	if (
			// 		std::string::npos != virtual_server.directives.at("server_name").find((*it2).directives.at("server_name")) ||
			// 		std::string::npos != (*it2).directives.at("server_name").find(virtual_server.directives.at("server_name")))
			// 		throw (std::runtime_error("Consig::...multiple servers"));
			(*it).sub_blocks.push_back(virtual_server);
			break ;
		}
	if (current.sub_blocks.end() == it)
	{
		t_conf_block	server(
			e_server_block,
			current.directives
		);
		server.sub_blocks.push_back(virtual_server);
		current.sub_blocks.push_back(server);
	}
}

void	Config::parse( t_conf_block& current ) { std::cout << "Config::parse()" << std::endl;

	std::string				cur_line;
	t_config_block_level	next_level;
	// t_conf_block			next_block;
	
	std::cout << "Parsing a " << current.level << std::endl;
	while (content_stream.good()) {
		std::getline(content_stream, cur_line);
		strip_trailing_and_leading_spaces(cur_line);
		std::cout << "read line: |" << cur_line << std::endl;
		
		if (std::string::npos != cur_line.find("{")) {
			next_level = next_conf_block_level(current.level);
			if (static_cast<int>(next_level) != block_get_level(cur_line))
				throw (std::runtime_error("Config::parse() : block found at wrong level"));
			std::cout << "next level: " << next_level << std::endl;
			if (e_server_block == next_level)
				parse_server_block(current);
			if (e_location_block == next_level)
				parse_location_block(current, cur_line);
			if (e_http_block == next_level)
				parse_http_block(current);
		}
		else if (std::string::npos != cur_line.find("}")) {
			if ("}" != cur_line)
				throw (std::runtime_error("Config::parse() : \'}\' must be on its own line"));
			return ;
		}
		else {
			std::stringstream	linestream(cur_line);
			std::string			key;
			std::string			value;
			// const std::string	keywords[] = {
			// 	"listen", "location", "server_name",
			// 	"index", "body_size", "error_page", "method",
			// 	"root", "autoindex", "exec_cgi", "extension_cgi"
			// };

			// size_t	i;
			// for (i = 0; i < 11; i++)
			// 	if (keywords[i] == key)
			// 		break ;
			// if (11 == i)
			// 	throw (std::runtime_error("illegal directive found"));
			std::getline(linestream, key, ' ');
			std::getline(linestream, value);
			current.directives[key] = value;
		}
	}
	// if (content_stream.bad()) {
	// 	throw (std::runtime_error("Config parse(): stream corrupted"));
	// }
	if (e_root_block != current.level)
		throw (std::runtime_error("Config parse(): mismatching brackets"));
	// return (current);
}




std::string type_get_name(t_config_block_level level){
	switch(level) {
		case e_root_block: return "root";
		case e_http_block: return "http";
		case e_server_block: return "server";
		case e_virtual_server_block: return "virtual server";
		case e_location_block: return "location";
	}
	return ("");
}

std::ostream&	operator<<(std::ostream& stream, const t_config_block_level& block) {

	stream << type_get_name(block);

	return (stream);
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




// /*brief*/	//* find any next block, and identifies the name 
// 			//* location blocks : returns the whole line not just name
// 			//* name is a single token stripped of spaces
// 			//* in case of location, it is location<path> (no space in between)
// std::pair<bool, std::string>	get_unknown_block_name(std::string& line, size_t i) {
// 	std::string	name;
// 	size_t		line_start, block_start, len;
	
// 	block_start = line.find("{", i);
// 	if (block_start == std::string::npos)
// 		return (std::make_pair(false, "get_unknown_block_name : no configuration block identified"));
	
// 	line_start = line.rfind("\n", block_start);
// 	if (line_start == std::string::npos)
// 		line_start = 0;
// 	else
// 		line_start++;
// 	len = block_start - line_start;// + 1;
// 	name = line.substr(line_start, len);
// 	name.erase(
// 		std::remove_if(name.begin(), name.end(), IsSpacePredicate()),
// 		name.end());
// 	return (std::make_pair(true, name));
// }

// /*brief*/   // find any next block
//             // if identifies next block and its level returns pair(level, block_name), if not identified returns pair(-1, error message)
//             // location blocks : returns the whole line not just name
// std::pair<int, std::string>	get_unknown_block_level(std::string& line, size_t i) {
// 	std::pair<int, std::string> level;

// 	std::pair<bool, std::string> ret = get_unknown_block_name(line, i);
// 	if ( ret.first == true ){
// 		level.first = type_get_level(ret.second);
// 		level.second = ret.second;
// 		if (level.first == -1)
// 			level.second = "get_unknown_block_level : unkown syntax for configuration block : " + level.second;
// 	}
// 	else{
// 		level.first = -1;
// 		level.second = ret.second;
// 	}
// 	return (level);
// }
