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

# include "include/webserv.hpp"
# include "ConnectionSocket.hpp"

//*	this class should be responsible for generating the response of a http Response.
class Response
{
private:
	const t_conf_block&			matching_directives;
	const ConnectionSocket&		connection;
	std::string					response;
public:
	//*		main constructors and destructors
						Response(
							const ConnectionSocket& connection,
							const t_conf_enclosing_block& enclosing_conf_block);

	//*		main functionalities
	void				generateResponse( void );
	const std::string&	getResponse( void );

	//*		Canonical Form Shit
						~Response();

private:
	//*		Helper functions
	const t_conf_block&			takeMatchingDirectives(const t_conf_enclosing_block& http_block);
};


#endif