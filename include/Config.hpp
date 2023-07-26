/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
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

# include "Webserv.hpp"

# define 	DEFAULT_PATHNAME   				"./configuration_files/default.conf"
# define	DEFAULT_CLIENT_MAX_BODY_SIZE	"1000000"			// 1M 1e+6 bytes
# define	LIMIT_CLIENT_MAX_BODY_SIZE		1000000000		// 1G 1e+9 bytes

class Config {
//*		Private member attributes ___________________________________
private:
	t_conf_block			conf;
	std::string			 	raw_content;
	std::stringstream	 	content_stream;

//*		Public member functions _____________________________________
public:
	//*	main Constructors and destructors
							Config (const char* config_file_path);
							~Config ( void );
	//*	main functionalities
	void					parse_config( void );
	const t_conf_block&		getConf( void );

//*		Private member functions ____________________________________
private:
	//* Unused canonical form
							Config( void );
							Config( const Config & sock );
	Config&					operator=(const Config& sock);
	//*	helper functions
	void					read_config_file(std::string conf_pathname);
	void					remove_comments( void );
	void					remove_empty_lines( void );
	void					parse( t_conf_block& current );
	void					parse_sub_block( t_conf_block& current, std::string& cur_line );
	void					parse_directive( t_conf_block& current, std::string& cur_line );
	void					check_directive_validity(const std::string& directive, t_config_block_level level);
	void					add_directive(t_conf_block& current, std::string& key, std::string& value);
	void					check_value_validity(std::string& key, std::string & value);
	void					check_value_validity_return(std::string & value);
	void					check_value_validity_body_size(std::string & value);
 	void					parse_server_block( t_conf_block& current );
	void					parse_location_block( t_conf_block& current, std::string& cur_line );
	void					parse_http_block( t_conf_block& current );
	void					set_up_default_values(t_conf_block& current);
};
#endif
