/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:26:23 by mmarinel          #+#    #+#             */
	/*   Updated: 2023/06/20 20:14:34 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Utils.hpp"
#include <algorithm>	//remove_if, std::replace
#include <cstring>		//strncmp
#include <set>			//check_directive_validity()

//*		non member helper functions Prototypes
static bool					parenthesis_balanced(const std::string& content);
static std::string			get_whole_line(std::string& line, size_t i);


//*		main Constructors and destructors
Config::		Config( const char* config_file_path ) : conf() {
	
	read_config_file(
		NULL == config_file_path ? DEFAULT_PATHNAME : config_file_path
	);
	content_stream.str(this->raw_content);
	COUT_DEBUG_INSERTION(FULL_DEBUG, "conf file: |" << this->raw_content << "|" << std::endl);
}

Config::		~Config( void ) {
}


//*		main Functionalities
void					Config::parse_config( void ) {

	//*		parsing

		//*		pre-processing
	if (false == parenthesis_balanced(this->raw_content))
		throw (std::invalid_argument("Config::parse_config: unbalanced parenthesis"));
	std::replace(this->raw_content.begin(), this->raw_content.end(), '\t', ' ');
		//*		parsing
	this->parse(this->conf);

	//*		validation
	if (this->conf.sub_blocks.empty())
		throw (std::invalid_argument("Config::parse_config() : no blocks in file"));
	const t_conf_block&	http = this->conf.sub_blocks[0];
	if (http.sub_blocks.empty())
		throw (std::invalid_argument("Config::parse_config() : no valid server block found"));
	else
		print_block(this->conf, this->conf.level);
}

const t_conf_block&		Config::getConf( void ) {
	return (this->conf);
}


//*		private helper functions
void	Config::read_config_file(std::string conf_pathname) {
	COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW << "read_config_file()" << RESET << std::endl);

	const size_t	buffer_size = 4096;
	char			buf[buffer_size + 1];
	int				bytes_read;
	
	int fd = open(conf_pathname.c_str(), O_RDONLY);
	if (fd == -1) {
		throw std::runtime_error("error open");
	}
	memset(buf, '\0', buffer_size + 1);
	while ((bytes_read = read(fd, buf, buffer_size)) > 0) {
		this->raw_content.append(buf);
		memset(buf, '\0', buffer_size + 1);
	}
	if (-1 == bytes_read) {
		if (-1 == close(fd))
			throw std::runtime_error("error read && close");
		throw std::runtime_error("error read");
	}
	else if (0 == bytes_read) {
		COUT_DEBUG_INSERTION(FULL_DEBUG, MAGENTA << "EOF Finished reading configuration file" << RESET << std::endl);
	}
	remove_comments();
	remove_empty_lines();
}

/*brief*/   // removes comment all comments = characters after a # and up until a \n
void	Config::remove_comments( void ) {
	COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW << "remove_comments()" << RESET << std::endl);
	
	size_t	nl_pos;
	
	for (size_t i = 0; (i = this->raw_content.find("#", i)) != std::string::npos;) {
		COUT_DEBUG_INSERTION(FULL_DEBUG, 
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
	COUT_DEBUG_INSERTION(FULL_DEBUG, BOLDYELLOW "Config::parse()" RESET << std::endl);

	std::string				cur_line;
	
	while (content_stream.good()) {
		COUT_DEBUG_INSERTION(FULL_DEBUG, 
			YELLOW "Parsing a " << current.level << " content" RESET << std::endl
		);
		std::getline(content_stream, cur_line);
		strip_trailing_and_leading_spaces(cur_line);
		COUT_DEBUG_INSERTION(FULL_DEBUG, 
			"line read: |" << cur_line << "|" << std::endl
		);
		
		if (std::string::npos != cur_line.find("{")) {
			parse_sub_block(current, cur_line);
		}
		else if (std::string::npos != cur_line.find("}")) {
			if ("}" != cur_line)
				throw (std::invalid_argument("Config::parse() : \'}\' must be on its own line"));
			if (current.invalidated)
				throw (std::invalid_argument("Config::parse() : current block invalidated (contained failed sub-block or invalid directive)"));
			return ;
		}
		else {
			parse_directive(current, cur_line);
		}
	}
	if (content_stream.bad())
		throw (std::runtime_error("Config::parse() : IO corrupted"));
}

void	Config::set_up_default_values(t_conf_block& current)
{
	if (current.directives.find("body_size") == current.directives.end())
		current.directives["body_size"] = DEFAULT_CLIENT_MAX_BODY_SIZE;	
}


void	Config::parse_sub_block( t_conf_block& current, std::string& cur_line )
{
	t_config_block_level	next_level;

	next_level = next_conf_block_level(current.level);
	COUT_DEBUG_INSERTION(FULL_DEBUG, "sub block found" << std::endl);
	COUT_DEBUG_INSERTION(FULL_DEBUG, "next level: " << next_level << std::endl);
	try {
		if (static_cast<int>(next_level) != block_get_level(cur_line))
			throw (std::invalid_argument("Config::parse_sub_block() : block found at wrong level"));
		if (e_server_block == next_level)
			parse_server_block(current);
		if (e_location_block == next_level)
			parse_location_block(current, cur_line);
		if (e_http_block == next_level)
			parse_http_block(current);

		set_up_default_values(current);

	}
	catch (const std::invalid_argument& e) {
		std::cout << BOLDRED
			<< "configuration block instantiation failed " << std::endl
			<< "containing block level : " << current.level << std::endl
			<< "level : " << next_level << std::endl
			<< "reason : " << e.what() << std::endl
			<< RESET;
		if (e_http_block != current.level) {
			COUT_DEBUG_INSERTION(FULL_DEBUG, "block invalidated" << std::endl);
			current.invalidated = true;
		}
		else {
			COUT_DEBUG_INSERTION(FULL_DEBUG, "block NOT invalidated" << std::endl);
		}
	}
}

void	Config::parse_http_block( t_conf_block& current ) {
	
	COUT_DEBUG_INSERTION(FULL_DEBUG, MAGENTA "parse_http_block()" RESET << std::endl);
	t_conf_block	http(
		e_http_block,
		current.directives
	);
	parse(http);
	COUT_DEBUG_INSERTION(FULL_DEBUG, 
		GREEN "pushing a new http block into root sub-blocks" RESET << std::endl
	);
	current.sub_blocks.push_back(http);
}

void	Config::parse_location_block( t_conf_block& current, std::string& cur_line  ) {
	
	COUT_DEBUG_INSERTION(FULL_DEBUG, MAGENTA "parse_location_block()" RESET << std::endl);
	
	std::stringstream	linestream(cur_line);
	std::string			location_keyword;
	std::string			location_path;

	//*	splitting direvtive name and value
	std::getline(linestream, location_keyword, ' ');
	std::getline(linestream, location_path, '{');
	strip_trailing_and_leading_spaces(location_keyword);
	strip_trailing_and_leading_spaces(location_path);
	t_conf_block		location(
		e_location_block,
		current.directives
	);
	location.directives[location_keyword] = location_path;
	
	//*	parsing block recursively
	parse(location);

	//*	check if location block already exists within the server
	//*		we add a fake directive in the server
	//*		with key location<location_path> and check for the existence of this key
	if (
		current.directives.end() != current.directives.find(location_keyword + location_path)
	)
		throw (std::invalid_argument("Config::parse_location_block() : found two conflicting location blocks"));
	else
		current.directives[location_keyword + location_path] = location_path;
	//**	////////////////////////////////////////////////////

	COUT_DEBUG_INSERTION(FULL_DEBUG, 
		GREEN "pushing a new location block into virtual server sub-blocks" RESET << std::endl
	);
	current.sub_blocks.push_back(location);
}

void	Config::parse_server_block( t_conf_block& current ) {
	COUT_DEBUG_INSERTION(FULL_DEBUG, MAGENTA "parse_server_block()" RESET << std::endl);
	
	t_conf_block&							http = current;
	std::vector<t_conf_block>::iterator		server;
	std::vector<t_conf_block>::iterator		virtual_serv;
	t_conf_block							new_virtual_serv(
		e_virtual_server_block,
		http.directives
	);

	//*	parsing block recursively
	parse(new_virtual_serv);

	//*	Checking block validity
	if (false == mandatory_server_directives_present(new_virtual_serv))
		throw (std::invalid_argument(
			"Config::parse_server_block() : server block should have \"listen\" directive")
		);
	for (server = http.sub_blocks.begin(); server != http.sub_blocks.end(); server ++)
		if (same_server(*server, new_virtual_serv))
		{
			for (
				virtual_serv = (*server).sub_blocks.begin();
				virtual_serv != (*server).sub_blocks.end();
				virtual_serv++
			)
				if (same_host(*virtual_serv, new_virtual_serv))
					throw (std::invalid_argument("Config::parse_server_block() : \
							 found two servers with conflicting server_names")
					);
			COUT_DEBUG_INSERTION(FULL_DEBUG, 
				GREEN "pushing a new virtual server block into existing server sub-blocks" RESET << std::endl
			);
			(*server).sub_blocks.push_back(new_virtual_serv);
			break ;
		}

	//*	New server block or virtual of existing one ?
	if (http.sub_blocks.end() == server)
	{
		t_conf_block	server(
			e_server_block,
			http.directives
		);
		COUT_DEBUG_INSERTION(FULL_DEBUG, 
			GREEN "pushing a new virtual server block into new server sub-blocks" RESET << std::endl
		);
		server.sub_blocks.push_back(new_virtual_serv);
		COUT_DEBUG_INSERTION(FULL_DEBUG, 
			GREEN "pushing new server block into http sub-blocks" RESET << std::endl
		);
		http.sub_blocks.push_back(server);
	}
	COUT_DEBUG_INSERTION(FULL_DEBUG, MAGENTA "parse_server_block() ----END" RESET << std::endl);
}

void	Config::parse_directive( t_conf_block& current, std::string& cur_line )
{
	std::stringstream	linestream;
	std::string			key;
	std::string			value;

	if (cur_line.empty())
		return ;
	try {
		//*		removing directive semicolon
		if (';' == cur_line[cur_line.length() - 1])
			cur_line = cur_line.substr(0, cur_line.length() - 1);
		else
			throw (std::invalid_argument("Config::parse_directive() : ';' missing"));
		//*		//////////////////////////////////////////////////

		COUT_DEBUG_INSERTION(FULL_DEBUG, "Parsing directive : " << cur_line << std::endl);
		linestream.str(cur_line);
		std::getline(linestream, key, ' ');
		std::getline(linestream, value);
		check_directive_validity(key, current.level);
		check_value_validity(key, value);
		add_directive(current, key, value);
	}
	catch (const std::invalid_argument& e) {
		std::cout << BOLDRED << "Config::parse_directive() : catched exception >> " << e.what() << RESET << std::endl;
		current.invalidated = true;
	}
}

void	Config::check_directive_validity(const std::string& directive, t_config_block_level level){
	COUT_DEBUG_INSERTION(FULL_DEBUG, " check_directive_validity | directive : " << directive << " | level : " << level << std::endl);

	static const char*					allowed_directives[] = {
		"listen", "location", "server_name", "host", "index", "body_size",
		"error_page", "method", "root", "upload_path", "return", "autoindex",
		"cgi_enable"
	};
	static const char*					virtual_server_directives[] = {
		"listen", "location", "server_name", "host", "index",
		"body_size", "error_page", "method", "root", "upload_path",
		"autoindex",  "cgi_enable"
	};
	static const char*					location_directives[] = {
		"location", "index", "body_size", "error_page", "method", "root",
		"upload_path", "return", "autoindex", "cgi_enable"
	};
	static const std::set<std::string>	dictionnary_all_directives(allowed_directives, allowed_directives + sizeof(allowed_directives) / sizeof(allowed_directives[0]));
	static std::set<std::string>		dictionnary_block_directives;
	size_t								dict_size;

	if (dictionnary_all_directives.count(directive) == 0)
		throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " is an unrecognized directive within the" + block_get_name(level) + " block."));

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
			throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " directive declared on the wrong scope ( declared in the" + block_get_name(level) + " block.)"));
    }

	dictionnary_block_directives = std::set<std::string>(block_directives, block_directives + dict_size);
	if (dictionnary_block_directives.count(directive) == 0)
		throw (std::invalid_argument("Config::check_directive_validity() : " +  directive  + " directive declared on the wrong scope ( declared in the" + block_get_name(level) + " block.)"));
}

void	Config::add_directive(t_conf_block& current, std::string& key, std::string& value)
{
	const std::string	singleValued = "listen body_size root upload_path return autoindex location";
	const std::string	multiValued = "server_name error_page method index cgi_enable";

	if (
		//*	directive already present
		current.directives.end() != current.directives.find(key) &&
		//*	directive is multi-valued
		str_compare_words(multiValued, key))
	{
		if (str_compare_words(current.directives.at(key), value))
			throw (std::invalid_argument(
				"Config::add_directive() : found directive with repeated values >>"
				+ key + "; " + value
				)
			);
		else
			current.directives[key].append(" " + value);
	}
	else
	{
		current.directives[key] = value;
	}
}

void	Config::check_value_validity(std::string& key, std::string & value)
{
	if ("body_size" ==  key)
		check_value_validity_body_size(value);
	if ("return" ==  key)
		check_value_validity_return(value);
}

// check if return directive is correct
// only one (optional) code, only one address 
// syntax -> return 307 http://example.com/new-page;
// syntax -> return http://example.com/new-page;

// 307	Temporary redirect		- Method and body not changed 
							//	- The Web page is temporarily unavailable for unforeseen reasons.
							//	- Better than 302 when non-GET operations are available on the site.
// 308	Permanent Redirect
							//	- Method and body not changed.
							//	- Reorganization of a website, with non-GET links/operations.
void	Config::check_value_validity_return(std::string & value)
{ COUT_DEBUG_INSERTION(FULL_DEBUG, YELLOW "check_value_validity_return()" RESET << std::endl);
    std::istringstream					iss(value);
    std::pair<std::string, std::string> val;
    std::string							word;
	size_t								status_code;

	// Get the first word
	if (!(iss >> val.first))
        throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : empty input."));
	// Get the second word (optional)
	if ((iss >> val.second))
	{
		// Check if there are more than two words in the directive
		if ((iss >> word))	
			throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : invalid config."));
	}

    // check code part of the directive
	// if status_code is not an int, it may be a url (as code is optional)
	if (std::string::npos != val.first.find_first_not_of("0123456789"))
	{
		if (val.second.empty() == false)
			throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : invalid config."));
		// if indeed a url -> manually correcting directive : updating default value for code (307)
		else					
			value = "307 " + val.first;
	}
	else
	{
		status_code = std::atol(val.first.c_str());
		if ( status_code < 307 || status_code > 308 )
			throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : invalid config (redirection code allowed : 307, 308)."));
	}
}

// check if body size directive is correct
// from 1 byte to 1e+9 bytes (1G) 
// 0 -> use default value // default value -> 1M 1e+6 bytes // limit value -> 1G 1e+9 bytes
// abbreviation accepted K and M  // 1K 1e+3 bytes // 1M 1e+6 bytes // 1G 1e+9 bytes
void	Config::check_value_validity_body_size(std::string & value)
{
	std::string body_size_value = value;

	// update body_size value if use of K or M abbreviation
	size_t pos = body_size_value.find_first_of("KM");
	if (pos != std::string::npos)
	{
		//*	if there is NO stuff after K/M
		if (body_size_value.size() == pos + 1){
			std::string magnitude = body_size_value[pos] == 'K' ? "000" : "000000";
			body_size_value.erase(pos);
			body_size_value += magnitude;

			value = body_size_value;
		}
		else 
			throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive."));
	}
	
	// check value validity using stoi (will throw if value isn't a number)
	if (std::string::npos != body_size_value.find_first_not_of("0123456789"))
		throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive."));
	size_t size = std::atol(body_size_value.c_str());
	if (size > LIMIT_CLIENT_MAX_BODY_SIZE)
		throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive (max 1G)."));
	if (size == 0)
		value = DEFAULT_CLIENT_MAX_BODY_SIZE;

	COUT_DEBUG_INSERTION(FULL_DEBUG, BOLDGREEN "body_size: " RESET << value << std::endl);
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
