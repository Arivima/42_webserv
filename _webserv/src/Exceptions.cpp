/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:37:05 by avilla-m          #+#    #+#             */
/*   Updated: 2023/07/13 18:16:32 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Exceptions.hpp"
# include "Utils.hpp"

# include <cstring>		// memset
# include <sstream>     // stringstream
# include <fstream>     // ifstream
# include <istream>     // std::getline
# include <cstdio>      // sprintf

// PUBLIC MEMBER FUNCTIONS
HttpError::HttpError(
	unsigned short		err_code,
	const t_conf_block&	matching_directives,
	const std::string&	location_root,
	const char *		errno_str
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


const char*			HttpError::what( void ) const throw() {
	char buf[5];

	memset(buf, '\0', 5);
	sprintf(buf, "%03hu ", err_code);
	return ((buf + std::string(" ") + msg + errno_str).c_str());
}


std::vector<char>	HttpError::getErrorPage( void ) const {
	return (err_page);
}


// PRIVATE MEMBER FUNCTIONS
std::string			HttpError::buildErrorPage( void )
{
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
			if (pageContentStream.is_open())
			{
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
		<< "Content-Length : " << page_content.length() << "\r\n"
		<< "\r\n";

	//*		append page content
	err_page << page_content;

	//*		return page
	return (err_page.str());
}


std::string			HttpError::errPage_getPath( void ) 
{
	COUT_DEBUG_INSERTION("errPage_getPath()" << std::endl);

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
	COUT_DEBUG_INSERTION("trying error page " << path << std::endl);
	if ("/" == path)
		return ("");
	return (path);
}


std::string			HttpError::defaultErrorPage( void )
{
	std::stringstream	defaultPageStream;

	defaultPageStream	<< "<!DOCTYPE html>"
						<< "<html lang=\"en\">"
							<< "<head>"
								<< "<meta charset=\"UTF-8\" />"
								<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
								<< "<title>"<< err_code << " " << msg << "</title>"
							<< "</head>"
							<< "<body>"
								<< "<center><h1>Webserv PiouPiou</h1><h2>" << err_code << " " << msg << "</br>" << errno_str << "</h2></center>"
							<< "</body>"
						<< "</html>";

	return (defaultPageStream.str());
}

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses

const char*			HttpError::takeMsg(unsigned short err_code)
{
	switch (err_code)
	{
		case 400:	return ("Bad Request");
		case 403:	return ("Forbidden");
		case 404:	return ("Not Found");
		case 405:	return ("Method Not Allowed");
		case 409:	return ("Conflict");
		case 413:	return ("Content Too Large");
		case 414:	return ("URI too long");
		case 500:	return ("Internal Server Error");
		case 501:	return ("Not Implemented");
		case 502:	return ("Bad Gateway");
		case 504:	return ("Gateway Timeout");
		default:	return ("Http Unknown Error");
	}
}
