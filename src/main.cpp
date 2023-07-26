/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Worker.hpp"
#include "Config.hpp"

int main(int ac, char** av)
{(void)ac;
	
	std::cout << BOLDYELLOW << "Welcome to Webserv" << RESET<< std::endl;
	try
	{
		Config	config(av[1]);
		
		//* parsing configuration file
		config.parse_config();
		COUT_DEBUG_INSERTION(FULL_DEBUG, "Configuration parsed\n");
		
		//* initializing worker
		Worker worker(config.getConf());
		COUT_DEBUG_INSERTION(FULL_DEBUG, "Worker initialized\n");
		
		//* starting worker
		worker.workerLoop();
		COUT_DEBUG_INSERTION(FULL_DEBUG, "Worker died without exceptions being thrown\n");
	}
	catch (std::exception& e)
	{
		std::cout << std::endl << BOLDRED "main : ! Caught exception >> " << e.what() << RESET << std::endl;
	}
	return (1);
}
