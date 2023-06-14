/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:43:27 by earendil          #+#    #+#             */
/*   Updated: 2023/06/14 16:36:01 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_HPP
# define TYPES_HPP

#include "include/webserv.hpp"

# include <cstdio>			//sprintf
# include <sys/socket.h>
# include <netinet/in.h>
# include <vector>			//config file data type sub-block

//*			Parsing

class ConnectionSocket;
class t_server_block;

typedef std::vector<ConnectionSocket* >		VectorCli;

typedef struct s_server {
	const t_server_block&		conf_server_block;
	int							server_fd;
	int							server_port;
	struct sockaddr_in			server_addr;
	socklen_t					server_addr_len;

	VectorCli					requests;
	
	s_server(const t_server_block& server_block) : conf_server_block(server_block) {}
}	t_server;

typedef std::vector<t_server>				VectorServ;


//*			Execution
//*	see https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses

class ClientError : public std::exception {
private:
	const unsigned short			err_code;
	const std::string				msg;
	
public:
	ClientError(
		unsigned short err_code = 400)
		: err_code(err_code), msg(takeMsg(err_code))
	{};
	virtual const char*	what( void ) const throw() {
		char buf[3];

		sprintf(buf, "%u ", err_code);
		return ((buf + msg).c_str());
	}
	const char*	takeMsg(unsigned short err_code) {
		switch (err_code) {
			case 400:
				return ("bad request");
			case 404:
				return ("not found");
			default:
				return ("bad request");
		}
	}
};

class ServerError : public std::exception {
private:
	const unsigned short			err_code;
	const std::string				msg;
	
public:
	ServerError(
		unsigned short err_code = 500)
		: err_code(err_code), msg(takeMsg(err_code))
	{};
	virtual const char*	what( void ) const throw() {
		char buf[3];

		sprintf(buf, "%u ", err_code);
		return ((buf + msg).c_str());
	}
	const char*	takeMsg(unsigned short err_code) {
		switch (err_code) {
			case 500:
				return ("internal server error");
			case 501:
				return ("not implemented");
			case 502:
				return ("Bad Gateway");
			default:
				return ("internal server error");
		}
	}
};

//*		Exceptions

class TaskFulfilled : public std::exception {
public:
	virtual const char*	what( void ) const throw() {
		return ("TaskFulfilled : switch connection status");
	}
};

class SockEof : public std::exception {
	public:
		virtual const char*	what( void ) const throw() {
			return ("Sockeof : client has left");
		}
};

class TimerExpired : public std::exception {
	public:
		virtual const char*	what( void ) const throw() {
			return ("TimerExpired : client starved");
		}
};

#endif