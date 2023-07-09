/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:09:56 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/09 23:15:12 by mmarinel         ###   ########.fr       */
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

# define DEFAULT_PATHNAME   "../_webserv/configuration_files/default.conf"
# define	DEFAULT_CLIENT_MAX_BODY_SIZE	1000000			// 1M 1e+6 bytes
# define	LIMIT_CLIENT_MAX_BODY_SIZE		1000000000		// 1G 1e+9 bytes

class Config {
private:
	t_conf_block			conf;
	std::string			 	raw_content;
	std::stringstream	 	content_stream;
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
	void					check_directive_validity(const std::string& directive, t_config_block_level level);
	void					add_directive(t_conf_block& current, std::string& key, std::string& value);
	void					check_value_validity(std::string& key, std::string & value);
	void					check_value_validity_body_size(t_conf_block& current);
 	void					parse_server_block( t_conf_block& current );
	void					parse_location_block( t_conf_block& current, std::string& cur_line );
	void					parse_http_block( t_conf_block& current );
	void					set_up_default_values(t_conf_block& current);
};

#endif