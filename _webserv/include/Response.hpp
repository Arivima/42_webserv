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
# include "Request.hpp"
# include "CGI.hpp"

/**
 * @brief this class is responsible for generating the response of a http request.
 * 
*/
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

	//*		MEMBER FIELDS
private:
	const t_conf_block&							matching_directives;
	Request*									request;				//*	request object
	const std::map<std::string, std::string>&	req;					//*	request headers map
	const std::string							location_root;
	const t_server&								assigned_server;
	const int									sock_fd;
	const std::string&							client_IP;				//*	ip of remote client
	const std::string&							server_IP;				//*	the interface, among all the assigned_server utilized interfaces, where the connection got accepted.
	const t_epoll_data&							edata;
	std::vector<char>							response;
	CGI* 										cgi;

public: 
	//*		main Constructors and Destructors
							Response(
								Request*			request,
								const t_server&		assigned_server,
								const int			sock_fd,
								const std::string&	client_IP,
								const std::string&	server_IP,
								const t_epoll_data&	edata
							);
							~Response();

	//*		main functionalities
	void					send_line( void );
	void					generateResponse( void );
	void					print_resp( void );

private:
	//*		Main Private Helper functions
	const t_conf_block&				takeMatchingDirectives(
										const t_conf_block& conf_server_block,
										const std::map<std::string, std::string>& req
									);
	const t_conf_block&				takeMatchingServer(
										const std::vector<t_conf_block>&	virtual_servers,
										const std::map<std::string, std::string>& req
									);
	size_t							locationMatch(
										const t_conf_block& location,
										const std::string& req_url
									);
	void							generateGETResponse( const std::string uri_path );
	void							generatePOSTResponse( const std::string uri_path );
	void							generateDELETEResponse( const std::string uri_path );
	std::string						getHeaders(
										int status, std::string description,
										std::string& filepath,
										size_t	body_size, std::string additional_header = ""
									);
	std::string						getIndexPage( const std::string& root, std::string path );

	//*		Secondary Helper Functions
	bool							isMethodAllowed(void);
	void							deleteFile( const std::string pathname );
	void							deleteDirectory( const std::string pathname );

	//TODO		forse move in utils
	std::string						http_req_complete_url_path(
										const std::string& uri,
										const std::string& root
									);
	void							throw_HttpError_errno_stat();
	std::string						take_location_root( void );

};


#endif