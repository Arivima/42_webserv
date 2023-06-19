/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:26:23 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/19 17:01:57 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <algorithm>	//remove_if
#include <cstring>		//strncmp

//TODO
//TODO	1. controllare che non ci siano più http block.
//TODO	N.B.: possiamo farlo direttamente in parenthesis_balanced()
//TODO	facendo in modo che quando scendiamo a 0 col count,
//TODO	non possiamo più né salire, né scendere.---------------DONE
//TODO
//TODO		multi-valued directives
//TODO	2. Sistemare l'append dei valori per error_page e server name(non fare overwrite)
//TODO
//TODO
//TODO	3. parse only accepted directives; define list of accepted directives.
//TODO
//TODO
//TODO

//TODO	add
//TODO			isKeyword
//TODO			directive_add (append or overwrite based on wether directive is multi-valued or not)
//*		non member helper functions Prototypes
static bool					parenthesis_balanced(const std::string& content);
static std::string			get_whole_line(std::string& line, size_t i);
//*		/////////////////////////////

//*		main Constructors and destructors
Config::		Config( const char* config_file_path ) : conf() {
	
	read_config_file(
		NULL == config_file_path ? DEFAULT_PATHNAME : config_file_path
	);
	content_stream.str(this->raw_content);
	COUT_DEBUG_INSERTION("conf file: |" << this->raw_content << "|" << std::endl);
}

Config::		~Config( void ) {
}


//*		main Functionalities
void					Config::parse_config( void ) {

	if (false == parenthesis_balanced(this->raw_content))
		throw (std::invalid_argument("Config::parse_config: unbalanced parenthesis"));
	this->parse(this->conf);
	print_block(this->conf, this->conf.level);
}

const t_conf_block&		Config::getConf( void ) {
	return (this->conf);
}


//*		private helper functions
void	Config::read_config_file(std::string conf_pathname) {
	COUT_DEBUG_INSERTION(YELLOW << "read_config_file()" << RESET << std::endl);

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
		COUT_DEBUG_INSERTION(MAGENTA << "EOF Finished reading configuration file" << RESET << std::endl);
	}
	remove_comments();
	remove_empty_lines();
}

/*brief*/   // removes comment all comments = characters after a # and up until a \n
void	Config::remove_comments( void ) {
	COUT_DEBUG_INSERTION(YELLOW << "remove_comments()" << RESET << std::endl);
	
	size_t	nl_pos;
	
	for (size_t i = 0; (i = this->raw_content.find("#", i)) != std::string::npos;) {
		COUT_DEBUG_INSERTION(
			"Found string to delete at pos : " << i << std::endl
			<<  RED << get_whole_line(this->raw_content, i) << RESET << std::endl
		);
		(void)get_whole_line;//*to avoid complaining when macro is not expanded
		
		nl_pos = this->raw_content.find("\n", i);
		if (std::string::npos == nl_pos)
			this->raw_content.erase(i);
		else
			this->raw_content.erase(i, nl_pos - i + 1);
	}
}

void	Config::remove_empty_lines(void) {

	size_t			line_start;
	size_t			line_end;
	std::string		line;
	
	line_start = 0;
	do {
		line_end = this->raw_content.find("\n", line_start);
		if (std::string::npos == line_end)
			line = this->raw_content.substr(line_start);
		else
			line = this->raw_content.substr(line_start, line_end - line_start + 1);
		if (std::string::npos == line.find_first_not_of(" 	\r\n\v\f")) {
			if (std::string::npos == line_end)
				this->raw_content.erase(line_start);
			else
				this->raw_content.erase(line_start, line_end - line_start + 1);
		}
		else {
			line_start = line_end;
			if (std::string::npos != line_start)
				line_start ++;
		}
	} while (std::string::npos != line_start && line_start < this->raw_content.length());
}

void	Config::parse( t_conf_block& current ) {
	COUT_DEBUG_INSERTION(BOLDYELLOW "Config::parse()" RESET << std::endl);

	std::string				cur_line;
	
	while (content_stream.good()) {
		COUT_DEBUG_INSERTION(
			YELLOW "Parsing a " << current.level << " content" RESET << std::endl
		);
		std::getline(content_stream, cur_line);
		strip_trailing_and_leading_spaces(cur_line);
		COUT_DEBUG_INSERTION(
			"line read: |" << cur_line << std::endl
		);
		
		if (std::string::npos != cur_line.find("{")) {
			parse_sub_block(current, cur_line);
		}
		else if (std::string::npos != cur_line.find("}")) {
			if ("}" != cur_line)
				throw (std::invalid_argument("Config::parse() : \'}\' must be on its own line"));
			return ;
		}
		else if (false == cur_line.empty()) {
			parse_directive(current, cur_line);
		}
	}
}

void	Config::parse_sub_block( t_conf_block& current, std::string& cur_line )
{
	t_config_block_level	next_level;

	next_level = next_conf_block_level(current.level);
	if (static_cast<int>(next_level) != block_get_level(cur_line))
		throw (std::invalid_argument("Config::parse() : block found at wrong level"));
	
	COUT_DEBUG_INSERTION("sub block found" << std::endl)
	COUT_DEBUG_INSERTION("next level: " << next_level << std::endl);
	if (e_server_block == next_level)
		parse_server_block(current);
	if (e_location_block == next_level)
		parse_location_block(current, cur_line);
	if (e_http_block == next_level)
		parse_http_block(current);
}

//TODO
//TODO	1. gestire multi-valued directives (vedere quali sono)
//TODO
//TODO	2. definire lista di directive accettate e rifiutare ogni directive non riconosciuta
//TODO
void	Config::parse_directive( t_conf_block& current, std::string& cur_line )
{
	std::stringstream	linestream;
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
	// 	throw (std::invalid_argument("illegal directive found"));
	
	//*		removing directive semicolon
	if (';' == cur_line[cur_line.length() - 1])
		cur_line = cur_line.substr(0, cur_line.length() - 1);
	//*		//////////////////////////////////////////////////

	COUT_DEBUG_INSERTION("Parsing directive : " << cur_line << std::endl);
	linestream.str(cur_line);
	std::getline(linestream, key, ' ');
	std::getline(linestream, value);
	current.directives[key] = value;
}

void	Config::parse_http_block( t_conf_block& current ) {
	
	COUT_DEBUG_INSERTION(MAGENTA "parse_http_block()" RESET << std::endl);
	t_conf_block	http(
		e_http_block,
		current.directives
	);
	parse(http);
	COUT_DEBUG_INSERTION(
		GREEN "pushing a new http block into root sub-blocks" RESET << std::endl
	);
	current.sub_blocks.push_back(http);
}

void	Config::parse_location_block( t_conf_block& current, std::string& cur_line  ) {
	
	COUT_DEBUG_INSERTION(MAGENTA "parse_location_block()" RESET << std::endl);
	
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
	parse(location);
	COUT_DEBUG_INSERTION(
		GREEN "pushing a new location block into virtual server sub-blocks" RESET << std::endl
	);
	current.sub_blocks.push_back(location);
}

void	Config::parse_server_block( t_conf_block& current ) {
	COUT_DEBUG_INSERTION(MAGENTA "parse_server_block()" RESET << std::endl);
	
	std::vector<t_conf_block>::iterator	it;
	std::vector<t_conf_block>::iterator	it2;
	t_conf_block						virtual_server(
		e_virtual_server_block,
		current.directives
	);
	parse(virtual_server);
	//*	current is http, it is a server.
	//*	Each server has list of virtual servers.
	for ( it = current.sub_blocks.begin(); it != current.sub_blocks.end(); it ++)
		if (
			(*it).sub_blocks[0].directives.at("listen") == virtual_server.directives.at("listen")
		)
		{
			for (it2 = (*it).sub_blocks.begin(); it2 != (*it).sub_blocks.end(); it2++)
				if (
					std::string::npos != virtual_server.directives.at("server_name").find((*it2).directives.at("server_name")) ||
					std::string::npos != (*it2).directives.at("server_name").find(virtual_server.directives.at("server_name"))
				)
					throw (std::invalid_argument("Config::parse_server_block() : multiple server port + server_name"));
			COUT_DEBUG_INSERTION(
				GREEN "pushing a new virtual server block into existing server sub-blocks" RESET << std::endl
			);
			(*it).sub_blocks.push_back(virtual_server);
			break ;
		}
	if (current.sub_blocks.end() == it)
	{
		t_conf_block	server(
			e_server_block,
			current.directives
		);
		COUT_DEBUG_INSERTION(
			GREEN "pushing a new virtual server block into new server sub-blocks" RESET << std::endl
		);
		server.sub_blocks.push_back(virtual_server);
		COUT_DEBUG_INSERTION(
			GREEN "pushing new server block into http sub-blocks" RESET << std::endl
		);
		current.sub_blocks.push_back(server);
	}
}


//*		non member helper functions
static bool					parenthesis_balanced(const std::string& content) {
	int							match_count;
	bool						http_block_closed;
	std::string::const_iterator	it;

	http_block_closed = false;
	match_count = 0;
	for (it = content.begin(); it != content.end(); it++)
	{
		if ('{' == (*it)) {
			match_count ++;
		}
		else
		if ('}' == (*it)) {
			match_count --;
			if (0 == match_count)
				http_block_closed = true;
		}
		
		if (match_count < 0)
			return (false);
		else if ( match_count > 0 && http_block_closed)
			throw (std::invalid_argument("multiple http blocks in configuration file"));
	}
	return (0 == match_count);
}

/*brief*/   // extracts and returns the whole line where i is positioned
static std::string			get_whole_line(std::string& line, size_t i) {
	size_t start, len;
	start = line.rfind("\n", i);
	if (start != std::string::npos)
		start++;
	start = (start != std::string::npos ? start : i);
	len = line.find("\n", i);
	len = (len != std::string::npos ? len : i) - start + 1;
	return (line.substr(start, len));
}
