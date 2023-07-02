/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:27:54 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/02 21:04:40 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Colors.hpp"
# include "Types.hpp"
# include "Exceptions.hpp"
// # include "../config_types.hpp"

// # ifndef DEBUG
// #  define DEBUG 0
// # endif

# ifdef DEBUG
#  define COUT_DEBUG_INSERTION(x) std::cout << x;
# else
#  define COUT_DEBUG_INSERTION(x)
# endif

# define DEFAULT_PORT_NUM 8080

# define	SIMPLE_HTML_RESP	\
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: 146\r\n\
Connection: close\r\n\
\r\n\
<!DOCTYPE html> \
<html> \
<head> \
  <title>Example HTTP Response</title> \
</head> \
<body> \
  <h1>Hello, World!</h1> \
  <p>This is an example HTTP response.</p> \
</body> \
</html>"
# define SIMPLE_HTML_RESP_SIZE	(sizeof(SIMPLE_HTML_RESP))

# define MAX_HTTP_REQ_LINE 8000
# define MAX_HTTP_HEAD_LINE 4096

//!	REMEMBER TO TOGLIEREEEE!!!!!!
# define RCV_BUF_SIZE 4//(MAX_HTTP_HEAD_LINE > MAX_HTTP_REQ_LINE ? MAX_HTTP_HEAD_LINE : MAX_HTTP_REQ_LINE)
// # define RCV_BUF_SIZE 1024
# define SND_BUF_SIZE 1024


//*		general purpose utilities
struct			IsSpacePredicate {
	bool operator()(char c) const {
		return std::isspace(static_cast<unsigned char>(c));
	}
};


//	BLOCK HPP
t_config_block_level		next_conf_block_level(t_config_block_level lvl);
std::ostream&				operator<<(std::ostream& stream, const t_config_block_level& block);
void						print_block(t_conf_block& block, size_t level);
void						print_directives(std::map<std::string, std::string>& directives, size_t level);
int							block_get_level(std::string block_name);
std::string 				block_get_name(t_config_block_level level);
bool						mandatory_server_directives_present(const t_conf_block& current);
bool						same_server( const t_conf_block& server, const t_conf_block& virtual_serv2);
bool						same_host( const t_conf_block& virtual_serv1, const t_conf_block& virtual_serv2);
std::string					take_location_root( const t_conf_block& matching_directives, bool file_upload );
bool						isCGI(
								const std::map<std::string, std::string>&	req,
								const t_conf_block&							matching_directives
							);
std::string					take_cgi_extension(
								const std::string& url,
								const std::map<std::string, std::string>& directives
							);
std::string					take_cgi_interpreter_path(
									const std::string& extension,
									const std::string& cgi_directive
							);

//	HTTP UTILS
std::string					uri_remove_queryString(const std::string& uri);

//	UTILS EXCEPTIONS
void throw_HttpError_debug(std::string function, std::string call, int httpStatusCode, const t_conf_block & matching_directives);

//	UTILS HPP
std::string					strip_spaces(std::string& str);
void						strip_trailing_and_leading_spaces(std::string& str);
std::vector<std::string> 	split_str_to_vector(std::string s, const std::string& delimiter);
/**
 * @brief this function takes two white-spaced words and returns true iff they have words in common
 * 
 * @param str_haystack 
 * @param str_needle 
 * @return true 
 * @return false 
 */
bool						str_compare_words(const std::string& str_haystack, const std::string& str_needle);
void						path_remove_leading_slash(std::string& pathname);
bool						isDirectory(const std::string root, std::string path, const t_conf_block& matching_directives);
std::string					getDirectoryContentList(const std::string directoryPath);
std::string					createHtmlPage(const std::string& body);
// std::string					get_cgi_extension(const std::string& path, const std::map<std::string, std::string>& directives);

#endif