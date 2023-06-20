/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:09:56 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/20 20:07:02 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <sstream>
# include <cstring>
# include <stdexcept>
# include <iostream>
# include <fcntl.h>
# include <unistd.h>

# include "include/webserv.hpp"
// # include "config_types.hpp"

# define DEFAULT_PATHNAME   "../_webserv/configuration_files/default.conf"

class Config {
private:
	t_conf_block		conf;
	std::string			raw_content;
	std::stringstream	content_stream;
public:
	//*	main Constructors and destructors
							Config (const char* config_file_path);
							~Config ( void );

	//*	main functionalities
	void					parse_config( void );
	const t_conf_block&		getConf( void );

private:
	//*	helper functions
	void					read_config_file(std::string conf_pathname);
	void					remove_comments( void );
	void					remove_empty_lines( void );
	void					parse( t_conf_block& current );
	void					parse_sub_block( t_conf_block& current, std::string& cur_line );
	void					parse_directive( t_conf_block& current, std::string& cur_line );
	
	//TODO	TODO	TODO	TODO	TODO	TODO	TODO	TODO	TODO	TODO	TODO	TODO
	bool					isDirective(const std::string& directive);
	void					add_directive(t_conf_block& current, std::string& key, std::string& value);
	//*	------------------------------------------------------------------------------------------------
	
	void					parse_server_block( t_conf_block& current );
	void					parse_location_block( t_conf_block& current, std::string& cur_line );
	void					parse_http_block( t_conf_block& current );
};

#endif