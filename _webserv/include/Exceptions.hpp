/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/01 18:14:36 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//! EXAMPLE
// class BLABLAException: public std::exception
// {
// 	public:
// 		virtual const char * what () const throw()
// 		{
// 			return ("BLABLA Error_message");
// 		}
// };

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

# include <iostream>
# include <string>
# include <stdexcept>

class SystemCallException: public std::exception
{
    private:
        SystemCallException();
    public:
        const std::string _sysCall;
        SystemCallException(const std::string & s) : _sysCall(s){}
        virtual const char * what () const throw(){return (("system call " + _sysCall + " failed.").c_str());}
};

class ConfigFileException: public std::exception
{
    private:
        ConfigFileException();
    public:
        const std::string _message;
        ConfigFileException(const std::string & s) : _message(s){}
        virtual const char * what () const throw(){return (("Configuration file error: " + _message + ".").c_str());}
};

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

//*		HttpError
//*	see https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses
# include <iostream>
# include <vector>
# include <map>
# include <string>
# include <cstring>//*	memset

# include <sstream>     // stringstream
# include <fstream>     // ifstream
# include <istream>     // std::getline
# include <cstdio>      //sprintf

class	HttpError : public std::exception {
private:
	std::vector<char>		err_page;
	const unsigned short	err_code;
	const std::string		msg;
	const t_conf_block&		matching_directives;
	const std::string		location_root;

public:
	HttpError(
		unsigned short      err_code,
		const t_conf_block& matching_directives,
		const std::string&	location_root
	)
	:	err_page(), err_code(err_code), msg(takeMsg(err_code)),
		matching_directives(matching_directives), location_root(location_root)
	{
		std::string         page_str = this->buildErrorPage();

		err_page.insert(
			err_page.begin(),
			page_str.begin(), page_str.end()
		);
	}

	virtual const char*	what( void ) const throw() {
		char buf[5];

		memset(buf, '\0', 5);
		sprintf(buf, "%03hu ", err_code);
		return ((buf + std::string(" ") + msg).c_str());
	}

	std::vector<char>	getErrorPage( void ) const {
		return (err_page);
	}
private:
	std::string		buildErrorPage( void ) {
		
		const std::map<std::string, std::string>	directives
			= this->matching_directives.directives;
		std::stringstream							err_page;
		std::string									page_content;
		std::string									line;

		//*		set page content
		if ( directives.end() != directives.find("error_page"))
		{
			std::string			err_page_path = errPage_getPath();

			if (false == err_page_path.empty())
			{
				std::ifstream		pageContentStream(this->location_root + err_page_path);

				page_content = "";
				if (pageContentStream.is_open()) {
					
					while (pageContentStream.good())
					{
						getline(pageContentStream, line);
						page_content += line;
					}
					if (pageContentStream.bad())
						page_content = defaultErrorPage();
				}
				else
					page_content = defaultErrorPage();
			}
			else
				page_content = defaultErrorPage();
		}
		else
			page_content = defaultErrorPage();

		//*		set headers
		err_page
			<< "HTTP/1.1 " << err_code << " " << msg << "\r\n"
			<< "Content-Type: text/html" << "\r\n"
			<< "Content-Length : " << page_content.length()
			<< "\r\n\r\n";

		//*		append page content
		err_page << page_content;

		//*		return page
		return (err_page.str());
	}

	std::string		errPage_getPath( void ) {std::cout << "errPage_getPath()" << std::endl;

		std::string									directive
			= this->matching_directives.directives.at("error_page");
		std::stringstream							directiveStream;
		std::stringstream							cur_error_stream;
		size_t										cur_error_pos;
		size_t										err_page_pos;
		std::string									path;

		//*		check if cur error matches error_page accepted errors
		cur_error_stream << this->err_code << " ";
		cur_error_pos = directive.find(cur_error_stream.str());
		if (std::string::npos == cur_error_pos)
			return ("");
		//*		take page for this error (take begin index inside string)
		err_page_pos = directive.find("/", cur_error_pos);
		if (std::string::npos == err_page_pos)
			return ("");
		//*		extract the page (path is at least "/")
		directiveStream.str(directive.substr(err_page_pos));
		std::getline(directiveStream, path, ' ');

		//*		return empty string in case of missing pathname
		std::cout << "trying error page " << path << std::endl;
		if ("/" == path)
			return ("");
		return (path);
	}

	std::string		defaultErrorPage( void ) {
		return (
			"<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
    <title>Default Error Page</title>\
  </head>\
  <body>\
    <h1>Default Error Page</h1>\
  </body>\
</html>\
"
		);
	}

	const char*	takeMsg(unsigned short err_code) {
		switch (err_code) {
			case 400:
				return ("bad request");
			case 404:
				return ("not found");
			case 500:
				return ("internal server error");
			case 501:
				return ("not implemented");
			case 502:
				return ("Bad Gateway");
			default:
				return ("http unknown error");
		}
	}
};

#endif