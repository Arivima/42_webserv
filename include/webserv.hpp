/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:27:54 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/05 17:35:02 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Colors.hpp"

# define MAX_HTTP_REQ_LINE 8000
# define MAX_HTTP_HEAD_LINE 4096

// # define RCV_BUF_SIZE (MAX_HTTP_HEAD_LINE > MAX_HTTP_REQ_LINE ? MAX_HTTP_HEAD_LINE : MAX_HTTP_REQ_LINE)
# define RCV_BUF_SIZE 1024

#endif