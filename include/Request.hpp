/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <map>						//*	parsed request (body included)
# include <string>
# include <sstream>					//*	socket stream
# include <ctime>					//* clock_gettime
# include <math.h>					//* pow

# include "Webserv.hpp"
# include "EpollData.hpp"

# define	DEFAULT_TIMEOUT		60	// 60.0 seconds

/**
 * @brief The purpose of this class is to parse a http request into a more convenient format.
 * When parsing is done, a fulfillment exception is thrown; after a fulfillment exception is thrown,
 * only getters may be used.
 * A fulfillment exception is an exception that signals work done.
 */
class Request {
//*		Private typedefs ____________________________________________
private:
	typedef enum e_PARSER_STATUS
	{
		e_READING_HEADS,
		e_READING_BODY,
	}	t_PARSER_STATUS;

//*		Private member attributes ___________________________________
private:
	t_PARSER_STATUS							parser_status;
	const int								sock_fd;
	const t_epoll_data&						edata;
	std::map<std::string, std::string>		req;						//*	map holding the request headers
	std::vector<char>						payload;					//*	request body
	struct timespec							timestamp_start;
	bool									timed_out;
	long long&								cur_memory_usage;			//*	cur memory usage of the whole webserv

	char									rcv_buf[RCV_BUF_SIZE + 1];	//*	buffer upon which we recv()
	std::vector<char>						sock_stream;				//*	vector on which we dump the request incoming data for a more convenient handling
	std::vector<char>						cur_line;					//*	current req line

	int										cur_body_size;				//*	remaining body bytes to be read
	bool									chunked;
	bool									next_chunk_arrived;
	size_t									cur_chunk_size;

//*		Public member functions _____________________________________
public:
	//*		main Constructors and Destructors
												Request(
													const int sock_fd,
													const t_epoll_data& edata,
													long long& cur_memory_usage
												);
												~Request( void );
	//*		main functionalities
	void										parse_line( void );
	const std::map<std::string, std::string>&	getRequest( void );
	const std::vector<char>&					getPayload( void );
	std::vector<char>							getIncomingData( void );
	bool										isChunked( void );
	bool										timedOut( void );
	//*		public helper functions
	void										print_req( void );

//*		Private member functions ____________________________________
private:
	//* Unused canonical form
					Request( void );
					Request( const Request& other );
	Request&		operator=( const Request& other );
	//*		Main helper functions
	void			read_line( void );
	void			read_header( void );
	void			read_body( void );
	void			parse_header( void );
	void			parse_req_line( std::vector<char>& req_line );
	void			parse_body( void );
	bool			isRequestTimeout();
	//*		Secondary helper functions
	void			printVectorChar(std::vector<char>& v, std::string name);
};
#endif