/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/17 12:52:22 by mmarinel         ###   ########.fr       */
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

#endif
