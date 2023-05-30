/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 17:56:31 by mmarinel          #+#    #+#             */
/*   Updated: 2023/05/30 13:45:19 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONSOCKET_HPP
# define CONNECTIONSOCKET_HPP

# include <cstdlib>
# include <cstring>
# include <sstream>
# include <iostream>
# include <fstream>

# include <algorithm>
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
	int										sock_fd;			//* connetction socket fd
	std::map<std::string, std::string>		headers;			//*	dictionary holding http req headers
	// std::filebuf							stream_buf;
	// FILE*									file;
	SocketStreamBuf							stream_buf;			//*	streambuf object needed by the istream object
	std::istream							stream;				//*	stream for handling socket reads
	std::string								cur_line;			//* current req line
	std::string								body;				//* current body
	bool									_cur_req_parsed;	//* flag which tells us if the current req parsing is done
	bool									_parse_body;		//*	flag which tells us if we have to parse the body (in which case,  we must read the next cur_body_size bytes)
	size_t									cur_body_size;

public:
	//*		main Constructors and Destructors
						ConnectionSocket( int sock_fd );

	//*		main Functions
	void				parse_line( void );
	void				send_response(const std::string& res);
	int					getSockFD( void );


	//*		CANONICAL FORM
						ConnectionSocket( void );
						~ConnectionSocket();
						ConnectionSocket( const ConnectionSocket & sock );
	ConnectionSocket&	operator=(const ConnectionSocket& sock);

	class				ParseError : public std::exception {
		public:
			virtual const char*	what( void ) const throw();
	};

private:
	t_PARSE_RET			read_line( void );
	// t_PARSE_RET			parse_body( void );
};


#endif