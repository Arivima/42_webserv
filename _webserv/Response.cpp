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
#include <sys/socket.h>	//send
#include <fstream>		//open requested files

Response::Response(
	const std::map<std::string, std::string>& req,
	const t_server& assigned_server,
	const int sock_fd,
	const t_epoll_data& edata)
		:
		req(req),
		assigned_server(assigned_server),
		sock_fd(sock_fd),
		edata(edata),
		matching_directives(takeMatchingDirectives(assigned_server.conf_server_block))
{
}

void	Response::generateResponse( void ) {
	if ("GET" == this->req.at("method"))
		return (generateGETResponse());
	// if ("POST" == this->req.at("method"))
	// 	return (generatePOSTResponse());
	// if ("PUT" == this->req.at("method"))
	// 	return (generatePUTResponse());
	// if ("DELETE" == this->req.at("method"))
	// 	return (generateDELETEResponse());
}

//TODO	TOMORROW
//TODO
//TODO	1.	refactor whole ConnectionSocket, make Request & Response classes
//TODO	2.	each class should throw an exception when task is complete so that ConnectionSocket
//TODO	can switch state with its switch-state functions
//TODO		2.1 make only one switch-state function, that switches to response or request
//TODO			depending on current status
//TODO	3.	when the ConnectionSocket catches a fulfillment exception, it should call the
//TODO		serve_client function again recursively (there will never be two consecutive
//TODO		recursive calls)
//*		....
//*		Done
//TODO	3.	make response field in Response class a string
//TODO		and use bytes_read to get position of first unsent byte, then trim the string
//TODO	4.	use SND_BUFFER_SIZE
//TODO
void	Response::send_line( void )
{
	size_t	bytes_read;

	if (response.empty())
		throw TaskFulfilled();
	else {
		bytes_read = send(this->sock_fd, response.c_str(), response.length(), 0);
		if (bytes_read < 0)
			throw ServerError();
		else
		if (0 == bytes_read)
			throw TaskFulfilled();
		else
			response = response.substr(bytes_read);
	}
}

void	Response::generateGETResponse( void )
{
	if (0 != req.at("url").find("http://"))
		throw ClientError();

	const std::string				root
		= this->matching_directives.directives.at("root");
	const std::string				reqPath(
		req.at("url").substr( req.at("url").substr(7).find("/") )
	);
	std::string						filePath(root + reqPath);
	std::ifstream					docstream(filePath.c_str());

	if (false == docstream.is_open())
		throw ClientError(404);
	response = "";
	while (docstream.good())
	{
		getline(docstream, response);
	}
	if (docstream.bad())
		throw ServerError();
}


//*		helper functions

//* 	prefix match implemented as of now
//TODO	see if we need to implement other kinds of match too 
bool	Response::locationMatch(
	const t_conf_block& location, const std::string& req_url
	)
{
	size_t	path_begin;

	return (
		0 == req_url.find("http://") &&
		std::string::n_pos != (path_begin = req_url.substr(7).find("/")) &&
		0 == req_url.substr(7 + path_begin).find(location.directives.at("location"))
	);
}

/**
 * @brief This function returns a handle to a generic block holding directives.
 * Given a server block, it returns the inner location block that matches
 * the current request, or the server block itself if no location block matches.
 * @param conf_server_block 
 * @return const t_conf_block& 
 */
const t_conf_block&	Response::takeMatchingDirectives(
	const t_conf_block& conf_server_block
	)
{
	const t_conf_block&							virtual_server 
		= takeMatchingServer(conf_server_block.sub_blocks);
	std::vector<t_conf_block>::const_iterator	location;
	
	for (
		location = virtual_server. sub_blocks.begin();
		location != virtual_server.sub_blocks.end();
		location ++
	)
	{
		if (locationMatch(*location, this->req.at("url")))
			break ;
	}

	if (virtual_server.sub_blocks.end() == location)
		return (virtual_server);
	else
		return (*location);
}

/**
 * @brief This function returns a handle to a generic block holding directives.
 * This function returns server block that matches
 * the current request.
 * 
 * @param virtual_servers 
 * @return const t_virtual_server_block& 
 */
const t_conf_block&	Response::takeMatchingServer(
		const std::vector<t_conf_block>&	virtual_servers
		)
{
	std::vector<t_conf_block>::const_iterator virtual_server;

	for (
		virtual_server = virtual_servers.begin();
		virtual_server != virtual_servers.end();
		virtual_server ++
	)
		if (
			(*virtual_server).\
				directives.at("server_name") == this->req.at("Host"))
			break ;
		
		//*		if no Host is matched, choose the default server
		return (
			virtual_server == virtual_servers.end() \
				? virtual_servers[0] \
				: (*virtual_server)
		);
}
