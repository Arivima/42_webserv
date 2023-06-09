/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:25:54 by earendil          #+#    #+#             */
/*   Updated: 2023/06/08 17:49:19 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(
	const std::map<std::string, std::string>& req,
	const t_server& assigned_server,
	int sock_fd,
	const t_epoll_data& edata)
		:
		req(req),
		assigned_server(assigned_server),
		sock_fd(sock_fd),
		edata(edata),
		matching_directives(takeMatchingDirectives(assigned_server.conf_server_block))
{
}

//*		helper functions

//* 	prefix match implemented as of now
//TODO	see if we need to implement other kinds of match too 
bool	Response::locationMatch(
	const t_location_block& location, const std::string& req_url
	)
{
	return (
		0 == req_url.find(location.directives.at("location"))
	);
}

const t_conf_block&	Response::takeMatchingDirectives(
	const t_server_block& conf_server_block
	)
{
	std::vector<t_location_block>::const_iterator	it;
	const t_virtual_server_block&					virtual_server 
		= takeMatchingServer(conf_server_block.virtual_servers);
	
	for (
		it = virtual_server.locations.begin();
		it != virtual_server.locations.end();
		it++
	)
	{
		if (locationMatch(*it, this->req.at("url")))
			break ;
	}

	if (virtual_server.locations.end() == it)
		return (virtual_server);
	else
		return (*it);
}

const t_virtual_server_block&	Response::takeMatchingServer(
		const std::vector<t_virtual_server_block>&	virtual_servers
		)
{
	std::vector<t_virtual_server_block>::const_iterator it;

	for ( it = virtual_servers.begin(); it != virtual_servers.end(); it ++ )
		if ((*it).directives.at("server_name") == this->req.at("Host"))
			break ;
		
		//*		if no Host is matched, choose the default server
		return (it == virtual_servers.end() ? virtual_servers[0] : (*it));
}
