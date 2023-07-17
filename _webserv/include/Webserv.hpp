/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:27:54 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/17 13:58:18 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define COUT_DEBUG_INSERTION(ON, x) \
    do { \
        if (ON) \
            std::cout << x; \
    } while (false)

# ifndef DEBUG
#  define DEBUG 0
# endif

# ifndef FULL_DEBUG
#  define FULL_DEBUG 0
# endif


# include "Colors.hpp"
# include "Types.hpp"
# include "Exceptions.hpp"

# define DEFAULT_PORT_NUM 8080

# define MAX_HTTP_REQ_LINE 8000
# define MAX_HTTP_HEAD_LINE 4096

# define RCV_BUF_SIZE (MAX_HTTP_HEAD_LINE > MAX_HTTP_REQ_LINE ? MAX_HTTP_HEAD_LINE : MAX_HTTP_REQ_LINE)

#endif