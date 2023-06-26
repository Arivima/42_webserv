/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/14 15:41:58 by earendil          #+#    #+#             */
/*   Updated: 2023/06/26 12:11:07 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <map>
# include <string>
# include <sstream>

# include "Webserv.hpp"
# include "EpollData.hpp"

/**
 * @brief The purpose of this class is to parse a http request into a more convenient format.
 * When parsing is done, a fulfillment exception is thrown; after a fulfillment exception is thrown,
 * only getters may be used.
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
	char									rcv_buf[RCV_BUF_SIZE + 1];	//*	
	std::stringstream						sock_stream;
	std::string								cur_line;					//*	current req line
	int										cur_body_size;

public:

	//*		main Constructors and Destructors
												Request(
													const int sock_fd,
													const t_epoll_data& edata
												);

	//*		main functionalities
	void										parse_line( void );
	const std::map<std::string, std::string>&	getRequest( void );
	void										print_req( void );

	//*		Canonical Form shit
					~Request( void );

private:

	//*		Canonical Form shit
					Request( void );
					Request( const Request& other );
	Request&		operator=( const Request& other );

	//*		helper functions
	void			read_line( void );
	void			parse_req_line( std::string& req_line );
	void			parse_header( void );
	void			parse_body( void );
	void			read_header( void );
	void			read_body( void );
};

#endif