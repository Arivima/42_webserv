/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 21:05:02 by earendil          #+#    #+#             */
/*   Updated: 2023/06/06 21:11:41 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//TODO
//*		1. split code of worker and syscall exception classes
//*		1. move everything good in _Exploration to _webserv

#include <iostream>         //	cin cout cerr
#include <stdio.h>         //	perror

#include "../_webserv/WorkerServer.hpp"

//*		Function Prototypes
int		ft_close(int fd);
void	ftError(std::string msg);

int main ()
{
	try
	{
		std::cout << "Welcome to mini-serv" << std::endl;
		WorkerServer server;
		server.serverLoop();
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl << BOLDRED ">>" << e.what() << RESET << std::endl;
	}
	return 0;
}

int ft_close(int fd) {
	std::cout << "Closing fd: " << fd << std::endl;
	return (close(fd));
}

void ftError(std::string msg) {
	perror(msg.c_str());
	std::cout << "Exiting mini-serv with failure" << std::endl;
	exit(EXIT_FAILURE);
}
