/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:27:54 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/20 17:34:23 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Colors.hpp"
# include "types.hpp"
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

# define RCV_BUF_SIZE (MAX_HTTP_HEAD_LINE > MAX_HTTP_REQ_LINE ? MAX_HTTP_HEAD_LINE : MAX_HTTP_REQ_LINE)
// # define RCV_BUF_SIZE 1024
# define SND_BUF_SIZE 1024


//*		general purpose utilities
struct			IsSpacePredicate {
	bool operator()(char c) const {
		return std::isspace(static_cast<unsigned char>(c));
	}
};
std::string		strip_spaces(std::string& str);
void			strip_trailing_and_leading_spaces(std::string& str);
/**
 * @brief this function takes two white-spaced words and returns true iff they have words in common
 * 
 * @param s1 
 * @param s2 
 * @return true 
 * @return false 
 */
//*		Mettere in utils.cpp !!!!!!!!!!!!!!!!!!!!!
bool			str_compare_words(const std::string& s1, const std::string& s2);
//*		/////////////////////////////////////////////////////////////


#endif