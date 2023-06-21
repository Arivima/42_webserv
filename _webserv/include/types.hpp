/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:43:27 by earendil          #+#    #+#             */
/*   Updated: 2023/06/21 17:18:01 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_HPP
# define TYPES_HPP

#include "webserv.hpp"

# include <string>
# include <sstream>
# include <iostream>		//cout
# include <fstream>			//ifstream
# include <cstdio>			//sprintf
# include <sys/socket.h>
# include <netinet/in.h>
# include <vector>			//config file data type sub-block and error_page in HttpError exception.
# include <map>

//*			Parsing
typedef enum e_config_block_level {
	e_root_block = 0,
	e_http_block,
	e_server_block,
	e_virtual_server_block,
	e_location_block,
}	t_config_block_level;

typedef struct s_conf_block {
    t_config_block_level				level;
    std::string							block_name;
	std::map<std::string, std::string>	directives;
    std::vector<struct s_conf_block>	sub_blocks;

	s_conf_block(
		t_config_block_level lvl = e_root_block,
		std::map<std::string, std::string> dir = std::map<std::string, std::string>()
	);
}	t_conf_block;

class ConnectionSocket;

typedef std::vector<ConnectionSocket* >		VectorCli;

typedef struct s_server {
	const t_conf_block&			conf_server_block;
	int							server_fd;
	int							server_port;
	struct sockaddr_in			server_addr;
	socklen_t					server_addr_len;

	VectorCli					requests;
	
	s_server(const t_conf_block& server_block) : conf_server_block(server_block) {}
}	t_server;

typedef std::vector<t_server>				VectorServ;


//*		type utilities
t_config_block_level	next_conf_block_level(t_config_block_level lvl);
int						block_get_level(std::string block_name);
std::string 			block_get_name(t_config_block_level level);
void					print_block(t_conf_block& block, size_t level);
void					print_directives(
	std::map<std::string, std::string>& directives,
	size_t level
	);
std::ostream&	operator<<(
	std::ostream& stream, const t_config_block_level& block
	);
//*		////////////////////////////////////////////////////


//*			Execution
//*	see https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses

//*		Exceptions

class	HttpError : public std::exception {
private:
	std::vector<char>		err_page;
	const unsigned short	err_code;
	const std::string		msg;
	const t_conf_block&		matching_directives;
	const std::string		location_root;

public:
	HttpError(
		unsigned short err_code,
		const t_conf_block& matching_directives,
		const std::string&	location_root
	)
	:	err_page(), err_code(err_code), msg(takeMsg(err_code)),
		matching_directives(matching_directives), location_root(location_root)
	{
		std::string		page_str = this->buildErrorPage();

		err_page.insert(
			err_page.begin(),
			page_str.begin(), page_str.end()
		);
	}

	virtual const char*	what( void ) const throw() {
		char buf[3];

		sprintf(buf, "%u ", err_code);
		return ((buf + std::string(" ") + msg).c_str());
	}

	std::vector<char>	getErrorPage( void ) const {
		return (err_page);
	}
private:
	std::string		buildErrorPage( void ) {
		
		const std::map<std::string, std::string>	directives
			= this->matching_directives.directives;
		std::stringstream							err_page;
		std::string									page_content;
		std::string									line;

		//*		set page content
		if ( directives.end() != directives.find("error_page"))
		{
			std::string			err_page_path = errPage_getPath();

			if (false == err_page_path.empty())
			{
				std::ifstream		pageContentStream(this->location_root + err_page_path);

				page_content = "";
				while (pageContentStream.good())
				{
					getline(pageContentStream, line);
					page_content += line;
				}
				if (pageContentStream.bad())
					page_content = defaultErrorPage();
			}
			else
				page_content = defaultErrorPage();
		}
		else
			page_content = defaultErrorPage();

		//*		set headers
		err_page
			<< "HTTP/1.1 " << err_code << " " << msg << "\r\n"
			<< "Content-Type: text/html" << "\r\n"
			<< "Content-Length : " << page_content.length()
			<< "\r\n\r\n";

		//*		append page content
		err_page << page_content;

		//*		return page
		return (err_page.str());
	}

	std::string		errPage_getPath( void ) {

		const std::map<std::string, std::string>	directives
			= this->matching_directives.directives;
		std::string									err_codes;
		std::string									path;
		std::stringstream							directiveStream(
			directives.at("error_page")
		);
		std::stringstream							cur_error_stream;

		cur_error_stream << this->err_code;
		std::getline(directiveStream, err_codes, '/');
		std::getline(directiveStream, path);

		//*		check if cur error matches error_page accepted errors
		if (path.empty() || std::string::npos == err_codes.find(cur_error_stream.str()))
			return ("");
		return (path);
	}

	std::string		defaultErrorPage( void ) {
		return (
			"<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
    <title>Default Error Page</title>\
  </head>\
  <body>\
    <h1>Default Error Page</h1>\
  </body>\
</html>\
"
		);
	}

	const char*	takeMsg(unsigned short err_code) {
		switch (err_code) {
			case 400:
				return ("bad request");
			case 404:
				return ("not found");
			case 500:
				return ("internal server error");
			case 501:
				return ("not implemented");
			case 502:
				return ("Bad Gateway");
			default:
				return ("http unknown error");
		}
	}
};

class TaskFulfilled : public std::exception {
public:
	virtual const char*	what( void ) const throw() {
		return ("TaskFulfilled : switch connection status");
	}
};

class SockEof : public std::exception {
	public:
		virtual const char*	what( void ) const throw() {
			return ("Sockeof : client has left");
		}
};

class TimerExpired : public std::exception {
	public:
		virtual const char*	what( void ) const throw() {
			return ("TimerExpired : client starved");
		}
};

#endif