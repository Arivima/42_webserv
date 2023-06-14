// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   ConnectionSocket.hpp                               :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2023/05/29 17:56:31 by mmarinel          #+#    #+#             */
// /*   Updated: 2023/06/09 16:31:56 by earendil         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #ifndef CONNECTIONSOCKET_HPP
// # define CONNECTIONSOCKET_HPP

// # include <cstdlib>
// # include <cstring>
// # include <sstream>
// # include <iostream>
// # include <fstream>
// # include <istream>
// # include <sys/socket.h>

// # include <algorithm>
// # include <string>
// # include <map>

// # include "include/webserv.hpp"
// # include "EpollData.hpp"
// # include "Response.hpp"

// //*	TBA: make both Request and Response classes here (We already have Response class)

// //*	this class should be responsible for holding a connection data (client, assigned server, request data, response data)
// //*	It should provide methods for both receiving connection data and sending response data
// //*	in a transparent way.
// class ConnectionSocket
// {
// public:
// 	//!	DEBUG
// 	int	flag;
// 	//!
// 	//*		Typedefs
// 	typedef enum e_REQUEST_STATUS
// 	{
// 		e_READING_REQ,
// 		e_SENDING_RESP,
// 	}	t_REQUEST_STATUS;
	
// 	typedef enum e_PARSER_MODE
// 	{
// 		e_READING_HEADS,
// 		e_READING_BODY,
// 	}	t_PARSER_MODE;
	
// private:
// 	const t_server&							assigned_server;
// 	int										sock_fd;					//*	connetction socket fd
// 	const t_epoll_data&						edata;						//*	epoll data reference
// 	t_REQUEST_STATUS						status;						//*	REQUEST or RESPONSE
// 	t_PARSER_MODE							parse_mode;					//*	HEADERS or BODY
// 	Response*								response;					//*	response data of current request
// 	std::map<std::string, std::string>		req;						//*	dictionary holding http req headers and body
// 	char									rcv_buf[RCV_BUF_SIZE + 1];	//*	
// 	std::stringstream						sock_stream;
// 	std::string								cur_line;					//*	current req line
// 	int										cur_body_size;

// public:
// 	//*		main Constructors and Destructors
// 							ConnectionSocket(
// 								int sock_fd,
// 								const t_server& assigned_server,
// 								const t_epoll_data& edata);
// 	//*		main Functions
// 	void										parse_line( void );
// 	// void										send_response( void );
// 	int											getSockFD( void );
// 	t_REQUEST_STATUS							getStatus( void );
// 	const std::map<std::string, std::string>&	getRequest( void );
// 	const t_server&								getAssignedServer( void ) const;
// 	void										print_req( void );

// 	//*		Exceptions
// 	class SockEof : public std::exception {
// 		public:
// 			virtual const char*	what( void ) const throw();
// 	};
// 	class LongHeader : public std::exception {
// 		public:
// 			virtual const char*	what( void ) const throw();
// 	};

// 	//*		CANONICAL FORM
// 						~ConnectionSocket();
// private:
// 						ConnectionSocket( void );
// 						ConnectionSocket( const ConnectionSocket & sock );
// 	ConnectionSocket&	operator=(const ConnectionSocket& sock);
	
// private:
// 	//*		helper functions
// 	void			read_line( void );
// 	void			parse_req_line( std::string& req_line );
// 	void			parse_header( void );
// 	void			parse_body( void );
// 	void			read_header( void );
// 	void			read_body( void );

// 	// void			req_mode_switch( void );
// 	void			resp_mode_switch( void );
// 	// void			set_directives( void );
// };


// #endif