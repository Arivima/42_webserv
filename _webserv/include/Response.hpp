/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:21:00 by earendil          #+#    #+#             */
/*   Updated: 2023/06/08 17:31:51 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Response_HPP
# define Response_HPP

# include <vector>
# include <map>
# include <string>

# include "Webserv.hpp"
# include "EpollData.hpp"

//*	this class should be responsible for generating the response of a http request.
class Response
{
private:
	const t_conf_block&							matching_directives;
	const std::map<std::string, std::string>	req;
	const t_server&								assigned_server;
	const int									sock_fd;
	const t_epoll_data&							edata;
	std::vector<char>							response;

public:
	//*		main constructors and destructors
							Response(
								const std::map<std::string, std::string>& req,
								const t_server& assigned_server,
								const int sock_fd,
								const t_epoll_data& edata);

	//*		main functionalities
	void					send_line( void );
	void					generateResponse( void );

	//*		Canonical Form Shit
							~Response();
	
private:
	//*		Private Helper functions
	void							generateGETResponse( void );
	std::string						getHeaders(
		int status, std::string description, std::string& filepath,
		size_t	body_size
	);
	bool							locationMatch(
		const t_conf_block& location, const std::string& req_url
		);
	const t_conf_block&				takeMatchingDirectives(
		const t_conf_block& conf_server_block,
		const std::map<std::string, std::string>& req
		);
	const t_conf_block&	takeMatchingServer(
		const std::vector<t_conf_block>&	virtual_servers,
		const std::map<std::string, std::string>& req
		);
	std::string			take_location_root( void );
	std::string			http_req_take_url_path(
		const std::string& url, const std::string& root
		);
	std::string			getIndexPage( const std::string& root );
};


#endif