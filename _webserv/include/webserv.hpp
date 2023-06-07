/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:27:54 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/07 17:28:19 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Colors.hpp"

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

#endif