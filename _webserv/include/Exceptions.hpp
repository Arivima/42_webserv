/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/13 17:26:44 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

# include "Webserv.hpp"
# include "Types.hpp"


class SystemCallException: public std::exception
{
    private:
        SystemCallException();
    public:
        const std::string _sysCall;
        SystemCallException(const std::string & s) : _sysCall(s){}
        virtual const char * what () const throw(){
			return (("system call " + _sysCall + " failed.").c_str());
		}
};

class ConfigFileException: public std::exception
{
    private:
        ConfigFileException();
    public:
        const std::string _message;
        ConfigFileException(const std::string & s) : _message(s){}
        virtual const char * what () const throw(){
			return (("Configuration file error: " + _message + ".").c_str());
		}
};

class TaskFulfilled : public std::exception
{
	public:
		virtual const char*	what( void ) const throw() {
			return ("TaskFulfilled : switch connection status");
		}
};

class SockEof : public std::exception
{
	public:
		virtual const char*	what( void ) const throw() {
			return ("Sockeof : client has left");
		}
};

class TimerExpired : public std::exception
{
	public:
		virtual const char*	what( void ) const throw() {
			return ("TimerExpired : client starved");
		}
};

class ChunkNotComplete : public std::exception
{
	public:
		virtual const char*	what( void ) const throw() {
			return ("Chunked Encoding : chunk not yet ready");
		}
};

class HttpError : public std::exception
{
// Private member attributes
private:
	std::vector<char>		err_page;
	const unsigned short	err_code;
	const std::string		msg;
	const t_conf_block&		matching_directives;
	const std::string		location_root;
	const std::string		errno_str;

// Public member functions
public:
	HttpError(
		unsigned short		err_code,
		const t_conf_block&	matching_directives,
		const std::string&	location_root,
		const char *		errno_str = NULL
	);

	virtual const char*		what( void ) const throw();
	std::vector<char>		getErrorPage( void ) const;

// Private member functions
private:
	std::string				buildErrorPage( void );
	std::string				errPage_getPath( void );
	std::string				defaultErrorPage( void );
	const char*				takeMsg(unsigned short err_code);
};

// HTTP Status codes
	// 400 Bad Request 				- The server cannot or will not process the request due to something that is perceived to be a client error 
	// 								(e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
	// 403 Forbidden 				- The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
	// 								Unlike 401 Unauthorized, the client's identity is known to the server.
	// 404 Not Found 				- The server cannot find the requested resource. In the browser, this means the URL is not recognized. 
	// 405 Method Not Allowed 		- The HyperText Transfer Protocol (HTTP) 405 Method Not Allowed response status code indicates that the server knows the request method, 
	//								but the target resource doesn't support this metho
	// 409 Conflict					- The deletion could not be completed due to a conflict with the current state of the resource. 
	// 413 Content Too Large		- The HTTP 413 Content Too Large response status code indicates that the request entity is larger than limits defined by server; 
	//								the server might close the connection or return a Retry-After header field.
	// 414 URI Too Long				- The URI requested by the client is longer than the server is willing to interpret.
	// 500 Internal Server Error	- An unexpected error occurred on the server while processing the deletion request.
	// 501 Not Implemented  		- The HyperText Transfer Protocol (HTTP) 501 Not Implemented server error response code means that the server 
	//								does not support the functionality required to fulfill the request.
	// 502 Bad Gateway 				- The HyperText Transfer Protocol (HTTP) 502 Bad Gateway server error response code indicates that the server, while acting as a gateway or proxy, 
	//								received an invalid response from the upstream server.
	// 504 Gateway Timeout 			- The HyperText Transfer Protocol (HTTP) 504 Gateway Timeout server error response code indicates that the server, while acting as a gateway or proxy, 
	//								did not get a response in time from the upstream server that it needed in order to complete the request.


#endif
