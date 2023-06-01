/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketStream.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/29 18:42:21 by mmarinel          #+#    #+#             */
/*   Updated: 2023/06/01 19:27:46 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETSTREAM_HPP
# define SOCKETSTREAM_HPP

# include <fstream>
# include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
# include <iostream>

class SocketStreamBuf : public std::streambuf {
private:
	static const int		bufferSize = 1024;
	int						socketFd;
	char					buffer[bufferSize];

public:
	//*		constructor
	explicit SocketStreamBuf(int socketFd);
	//*		main functionalities
	bool	has_eol( void );
protected:
	int_type underflow( void );
};

#endif