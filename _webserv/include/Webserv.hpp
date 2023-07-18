/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
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
#  ifdef FULL_DEBUG
#   define DEBUG 1
#  else
#   define DEBUG 0
#  endif
# endif

# ifndef FULL_DEBUG
#  define FULL_DEBUG 0
# endif


# include "Colors.hpp"
# include "Types.hpp"
# include "Exceptions.hpp"

//*	in bytes
# define MAX_MEMORY_USAGE	(1024 * 1024 * 1024 * 1.5f)
//* 4GB

# define DEFAULT_PORT_NUM 8080

# define MAX_HTTP_REQ_LINE 8000
# define MAX_HTTP_HEAD_LINE 4096

# define RCV_BUF_SIZE (MAX_HTTP_HEAD_LINE > MAX_HTTP_REQ_LINE ? MAX_HTTP_HEAD_LINE : MAX_HTTP_REQ_LINE)

#endif