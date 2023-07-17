/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/17 16:59:53 by mmarinel         ###   ########.fr       */
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
	explicit HttpError(
		unsigned short		err_code,
		const t_conf_block&	matching_directives,
		const std::string&	location_root,
		const char *		errno_str = NULL
	);
	virtual ~HttpError() throw() ;

	virtual const char*		what( void ) const throw();
	std::vector<char>		getErrorPage( void ) const;

// Private member functions
private:
	std::string				buildErrorPage( void );
	std::string				errPage_getPath( void );
	std::string				defaultErrorPage( void );
	const char*				takeMsg(unsigned short err_code);
};

#endif
