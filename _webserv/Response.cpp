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
		const ConnectionSocket& connection,
		const t_conf_enclosing_block& enclosing_conf_block)
		:
		connection(connection),
		matching_directives(takeMatchingDirectives(enclosing_conf_block))
{
}

//*		helper functions

const t_conf_block&	Response::takeMatchingDirectives(
	const t_conf_enclosing_block& enclosing_conf_block
	)
{
	const t_http_block&	http_block = enclosing_conf_block.http;
	for (
		std::vector<t_server_block>::const_iterator it = http_block.servers.begin();
		it != http_block.servers.end();
		it++)
	{
		if (
			this->connection.getAssignedServer().directives.at("port")\
			!=\
			(*it).directives.at("port")
		)
			return (*it);
	}
	return (enclosing_conf_block);
}
