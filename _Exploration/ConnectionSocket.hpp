/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 17:56:31 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/29 19:07:08 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONSOCKET_HPP
# define CONNECTIONSOCKET_HPP

# include <sstream>
# include <iostream>
# include <fstream>

# include <string>
# include <map>

# include "SocketStream.hpp"

class ConnectionSocket
{
public:
	//*		Typedefs
	typedef enum e_PARSE_RET
	{
		e_ERR_PARSE,
		e_OK_PARSE,
		e_CONTINUE_PARSE
	}	t_PARSE_RET;

	typedef enum e_HTTP_METHOD
	{
		e_GET,
		e_POST,
		e_UPDATE,
		e_PUT,
		e_DELETE
	}	t_HTTP_METHOD;

private:
	int										sock_fd;
	std::map<std::string, std::string>		headers;
	// std::filebuf							stream_buf;
	// FILE*									file;
	SocketStreamBuf							stream_buf;
	std::istream							stream;
	std::string								cur_line;

public:
	//*		main Constructors and Destructors
						ConnectionSocket( int sock_fd );

	//*		main Functions
	t_PARSE_RET			parse_line( void );
	void				send_response(const std::string& res);
	int					getSockFD( void );


	//*		CANONICAL FORM
						ConnectionSocket( void );
						~ConnectionSocket();
						ConnectionSocket( const ConnectionSocket & sock );
	ConnectionSocket&	operator=(const ConnectionSocket& sock);
};


#endif