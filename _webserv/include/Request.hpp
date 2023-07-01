/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/14 15:41:58 by earendil          #+#    #+#             */
/*   Updated: 2023/07/01 16:26:30 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <map>//*	parsed request (body included)
# include <string>
# include <sstream>//*	socket stream

# include "Webserv.hpp"
# include "EpollData.hpp"


//TODO
//TODO	1.	handle Chuncked Encoding (Transfer-Encoding header)
//TODO	2.	(optional) handle binary data in body
//TODO

/**
 * @brief The purpose of this class is to parse a http request into a more convenient format.
 * When parsing is done, a fulfillment exception is thrown; after a fulfillment exception is thrown,
 * only getters may be used.
 * A fulfillment exception is an exception that signals work done.
 */
class Request {
private:

	typedef enum e_PARSER_STATUS
	{
		e_READING_HEADS,
		e_READING_BODY,
	}	t_PARSER_STATUS;

	t_PARSER_STATUS							parser_status;
	const int								sock_fd;
	const t_epoll_data&						edata;
	std::map<std::string, std::string>		req;
	char									rcv_buf[RCV_BUF_SIZE + 1];	//*	buffer upon which we recv()
	std::stringstream						sock_stream;				//*	stream where we dump all the data read in rcv buffer for a more convenient handling
	std::string								cur_line;					//*	current req line
	int										cur_body_size;				//*	remaining body bytes to be read

public:

	//*		main Constructors and Destructors
												Request(
													const int sock_fd,
													const t_epoll_data& edata
												);
												~Request( void );
	
	//*		main functionalities
	void										parse_line( void );
	const std::map<std::string, std::string>&	getRequest( void );
	void										print_req( void );

private:

	//*		Canonical Form shit
					Request( void );
					Request( const Request& other );
	Request&		operator=( const Request& other );

	//*		helper functions
	void			read_line( void );
	void			read_header( void );
	void			read_body( void );
	void			parse_header( void );
	void			parse_req_line( std::string& req_line );
	void			parse_body( void );
};

#endif