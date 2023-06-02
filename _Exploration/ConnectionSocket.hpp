/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionSocket.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 17:56:31 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/02 11:05:41 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONSOCKET_HPP
# define CONNECTIONSOCKET_HPP

# include <cstdlib>
# include <cstring>
# include <sstream>
# include <iostream>
# include <fstream>
# include <istream>

# include <algorithm>
# include <string>
# include <map>

# include "EpollData.hpp"
# include "SocketStream.hpp"

class ConnectionSocket
{
public:
	//!	DEBUG
	int	flag;
	//!
	//*		Typedefs
	typedef enum e_CLIENT_STATUS
	{
		e_READ_MODE,
		e_RESP_MODE,
	}	t_CLIENT_STATUS;
	
	typedef enum e_PARSER_MODE
	{
		e_READ_HEADS,
		e_READ_BODY,
	}	t_PARSER_MODE;
	
private:
	int										sock_fd;			//*	connetction socket fd
	const t_epoll_data&						edata;				//*	epoll data reference
	t_CLIENT_STATUS							status;				//*	REQUEST or RESPONSE
	t_PARSER_MODE							parse_mode;			//*	HEADERS or BODY
	std::map<std::string, std::string>		req;				//*	dictionary holding http req headers
	SocketStreamBuf							stream_buf;			//*	stream_buf object for the input stream
	std::istream							stream;				//*	stream for handling socket reads
	std::string								cur_line;			//*	current req line
	int										cur_body_size;

public:
	//*		main Constructors and Destructors
						ConnectionSocket( int sock_fd, const t_epoll_data& edata );
	//*		main Functions
	void				parse_line( void );
	void				send_response(const std::string& res);
	int					getSockFD( void );
	bool				req_done( void );

	//*		Exceptions
	class SockEof : public std::exception {
		public:
			virtual const char*	what( void ) const throw();
	};

	//*		CANONICAL FORM
						~ConnectionSocket();
private:
						ConnectionSocket( void );
						ConnectionSocket( const ConnectionSocket & sock );
	ConnectionSocket&	operator=(const ConnectionSocket& sock);
	
private:
	//*		helper functions
	void			read_line( void );
};


#endif