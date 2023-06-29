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
// # include "CGI.hpp"

//*	this class should be responsible for generating the response of a http request.
class Response
{
	//*		TYPEDEFS (private)
private:
	/**
	 * @brief this structure represents location matching data.
	 * It has a variable "match_score" indicating the number of characters matching
	 * the location block path. This variable is npos (maximum) for exact matches.
	 * 
	 */
	typedef struct s_location_match
	{
		const t_conf_block&	location;
		size_t				match_score;

		s_location_match(const t_conf_block& location, size_t match_score)
		: location(location)
		{
			this->match_score = match_score;
		}
		bool	operator<(const s_location_match& other) {
			return (this->match_score < other.match_score);
		}
	}	t_location_match;

private:
	const t_conf_block&							matching_directives;
	const std::map<std::string, std::string>	req;
	const t_server&								assigned_server;
	const int									sock_fd;
	const std::string&							client_IP;				//*	ip of remote client
	const std::string&							server_IP;				//*	the interface, among all the assigned_server utilized interfaces, where the connection got accepted.
	const t_epoll_data&							edata;
	std::vector<char>							response;
	// CGI 										cgi;

public: 
	//*		main constructors and destructors
							Response(
								const std::map<std::string, std::string>& req,
								const t_server& assigned_server,
								const int sock_fd,
								const std::string& client_IP,
								const std::string& server_IP,
								const t_epoll_data& edata);

	//*		main functionalities
	void					send_line( void );
	void					generateResponse( void );

	//*		Canonical Form Shit
							~Response();
	
private:
	//*		Private Helper functions
	void							generateGETResponse( void );
	void							GETStatic( const std::string& reqRelativePath );
	void							generatePOSTResponse( void );
	// void							generateDELETEResponse( void );
	std::string						getHeaders(
		int status, std::string description, std::string& filepath,
		size_t	body_size
	);
	//TODO		forse move in utils
	size_t							locationMatch(
		const t_conf_block& location, const std::string& req_url
		);
	const t_conf_block&				takeMatchingDirectives(
		const t_conf_block& conf_server_block,
		const std::map<std::string, std::string>& req
		);
	const t_conf_block&				takeMatchingServer(
		const std::vector<t_conf_block>&	virtual_servers,
		const std::map<std::string, std::string>& req
		);
	//TODO		forse move in utils
	std::string						http_req_complete_url_path(
		const std::string& url, const std::string& root
		);
	std::string						getIndexPage( const std::string& root, std::string path );
};


#endif