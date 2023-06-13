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

# include <string>
# include <sstream>

# include "include/webserv.hpp"
# include "EpollData.hpp"

//*	this class should be responsible for generating the response of a http request.
class Response
{
private:
	const t_conf_block&							matching_directives;
	const std::map<std::string, std::string>&	req;
	const t_server&								assigned_server;
	const int									sock_fd;
	const t_epoll_data&							edata;
	std::string									response;
public:
	//*		main constructors and destructors
							Response(
								const std::map<std::string, std::string>& req,
								const t_server& assigned_server,
								int sock_fd,
								const t_epoll_data& edata);

	//*		main functionalities
	void					send_line( void );
	void					generateResponse( void );
	const std::string&		getResponse( void );

	//*		Canonical Form Shit
							~Response();
	
	//*		fulfillment
	class ResponseFulfilled : public TaskFulfilled {
	public:
		virtual const char*	what( void ) const throw() {
			return ("response fulfilled : switch to request mode");
		}
	};

private:
	//*		Helper functions
	void							generateGETResponse( void );
	bool							locationMatch(
		const t_location_block& location, const std::string& req_url
		);
	const t_conf_block&				takeMatchingDirectives(
		const t_server_block& conf_server_block
		);
	const t_virtual_server_block&	takeMatchingServer(
		const std::vector<t_virtual_server_block>&	virtual_servers
		);
};


#endif