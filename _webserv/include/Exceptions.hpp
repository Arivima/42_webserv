/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/03 16:54:09 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	const std::string		errno_str;

public:
	HttpError(
		unsigned short		err_code,
		const t_conf_block&	matching_directives,
		const std::string&	location_root,
		const char *		errno_str = NULL
	)
	:	err_page(), err_code(err_code), msg(takeMsg(err_code)),
		matching_directives(matching_directives), location_root(location_root), 
		errno_str(errno_str? (std::string(" " + std::string(errno_str))) : std::string(""))
	{
		std::string		page_str = this->buildErrorPage();

		err_page.insert(
			err_page.begin(),
			page_str.begin(), page_str.end()
		);
	}

	virtual const char*	what( void ) const throw() {
		char buf[5];

		memset(buf, '\0', 5);
		sprintf(buf, "%03hu ", err_code);
		return ((buf + std::string(" ") + msg + errno_str).c_str());
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
		std::stringstream	defaultPageStream;

		defaultPageStream << "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
    <title>"<< err_code << " " << msg << "--" << errno_str << "</title>\
  </head>\
  <body>\
    <h1>"<< err_code << " " << msg << "--" << errno_str << "</h1>\
  </body>\
</html>\
";

		return (defaultPageStream.str());
	}

	const char*	takeMsg(unsigned short err_code) {
		switch (err_code) {
			case 400:
				return ("Bad Request");
			case 403:
				return ("Forbidden");
			case 404:
				return ("Not Found");
			case 409:
				return ("Conflict");
			case 414:
				return ("URI too long");
			case 500:
				return ("Internal Server Error");
			case 501:
				return ("Not Implemented");
			case 502:
				return ("Bad Gateway");
			default:
				return ("Http Unknown Error");
		}
	}
};

#endif


// HTTP Status codes
	// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.
	// 400 Bad Request 		- The server cannot or will not process the request due to something that is perceived to be a client error 
	// 						(e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
	// 403 Forbidden 		- The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
	// 						Unlike 401 Unauthorized, the client's identity is known to the server.
	// 404 Not Found 		- The server cannot find the requested resource. In the browser, this means the URL is not recognized. 
	// 409 Conflict			- The deletion could not be completed due to a conflict with the current state of the resource. 
	// 414 URI Too Long		- The URI requested by the client is longer than the server is willing to interpret.
	// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.




// switch(errno) {
// 	// 400 Bad Request //? Empty because directory validity (client input) has been previously checked
	
// 	// 403 Forbidden - The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
// 	case EACCES:	throw HttpError(403, matching_directives, root, strerror(errno)); 
// 	case EPERM:		throw HttpError(403, matching_directives, root, strerror(errno)); 
// 	case EROFS:		throw HttpError(403, matching_directives, root, strerror(errno)); 
	
// 	// 409 Conflict: The deletion could not be completed due to a conflict with the current state of the resource. 
// 	case EBUSY:		throw HttpError(409, matching_directives, root, strerror(errno)); 
	
// 	// 414 url too long //? Empty because directory validity (client input) has been previously checked
	
// 	// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.
// 	case ENOMEM:	throw HttpError(500, matching_directives, root, strerror(errno)); 
// 	case EOVERFLOW:	throw HttpError(500, matching_directives, root, strerror(errno)); 
// 	case ENOENT :	throw HttpError(500, matching_directives, root, strerror(errno));
// 	case ENOTEMPTY:	throw HttpError(500, matching_directives, root, strerror(errno)); 
// 	case ENOTDIR:	throw HttpError(500, matching_directives, root, strerror(errno));  
// 	case EINVAL:	throw HttpError(500, matching_directives, root, strerror(errno));  
// 	case EFAULT:	throw HttpError(500, matching_directives, root, strerror(errno));  
// 	case ELOOP :	throw HttpError(500, matching_directives, root, strerror(errno)); 
// 	case ENAMETOOLONG:throw HttpError(500, matching_directives, root, strerror(errno)); 
	
// 	// 404 Not Found //? Empty because directory validity (client input) has been previously checked
	
// 	// default case : EMFILE, ENFILE, EBADF
// 	default:		throw HttpError(500, matching_directives, root, strerror(errno)); 
// }

// opendir ERRORS : 
    //    EACCES Permission denied.
    //    EBADF  fd is not a valid file descriptor opened for reading.
    //    EMFILE The per-process limit on the number of open file descriptors has been reached.
    //    ENFILE The system-wide limit on the total number of open files has been reached.
    //    ENOENT Directory does not exist, or name is an empty string.
    //    ENOMEM Insufficient memory to complete the operation.
    //    ENOTDIR name is not a directory.
// rmdir ERRORS : 
    //    EACCES Write access to the directory containing pathname was not allowed, or one of the directories in the path prefix of
    //           pathname did not allow search permission.  (See also path_resolution(7).)
    //    EBUSY  pathname is currently in use by the system or some process that prevents its removal.  On Linux, this means pathname
    //           is currently used as a mount point or is the root directory of the calling process.
    //    EFAULT pathname points outside your accessible address space.
    //    EINVAL pathname has .  as last component.
    //    ELOOP  Too many symbolic links were encountered in resolving pathname.
    //    ENAMETOOLONG pathname was too long.
    //    ENOENT A directory component in pathname does not exist or is a dangling symbolic link.
    //    ENOMEM Insufficient kernel memory was available.
    //    ENOTDIR pathname, or a component used as a directory in pathname, is not, in fact, a directory.
    //    ENOTEMPTY pathname contains entries other than . and .. ; or, pathname has ..  as its final component.
	//    			POSIX.1 also allows EEXIST for this condition.
    //    EPERM  The directory containing pathname has the sticky bit (S_ISVTX) set and the process's effective user ID is
    //           neither the user ID of the file to be deleted nor that of the directory containing it, and the process is not
    //           privileged (Linux: does not have the CAP_FOWNER capability).
    //    EPERM  The filesystem containing pathname does not support the removal of directories.
    //    EROFS  pathname refers to a directory on a read-only filesystem.
