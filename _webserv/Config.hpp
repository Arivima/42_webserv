/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/16 14:09:56 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/16 17:47:14 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <cstring>
# include <stdexcept>
# include <iostream>
# include <fcntl.h>
# include <unistd.h>

# include "include/webserv.hpp"
# include "config_types.hpp"

# define DEFAULT_PATHNAME   "../_webserv/configuration_files/default.conf"

class Config {
private:
	t_conf_block	conf;
	std::string		raw_content;
public:
	//*	main Constructors and destructors
							Config (const char* config_file_path);
							~Config ( void );

	//*	main functionalities
	void					parse( void );
	const t_conf_block&		getConf( void );

private:
	//*	helper functions
	void					read_config_file(std::string conf_pathname);
	void					remove_comments( void );
};

#endif