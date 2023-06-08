/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 13:20:37 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/08 16:32:55 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# define DEBUG          1

#include "include/Colors.hpp"
#include "Exceptions.hpp"
#include "Worker.hpp"
#include "Parsing_config_file.hpp"


int main(int ac, char** av){
    std::cout << "Welcome to mini-serv" << std::endl;
    try
    {
        //* parsing configuration file
        if (ac == 1){
            // initialization_default();
            ;
        }
        else if (ac == 2) {
            parse_configuration_file(av[1]);
            // initialization_configuration_file();
            ;
        }
        else
            // throw std::exception("Wrong number of arguments.\n ./webserv [configuration_file]\n");
            ;
        
        //* initializing server
        // Worker server;
        
        //* starting worker
        // server.workerLoop();
    }
	catch (std::exception& e)
    {
		std::cout << std::endl << BOLDRED "! Caught exception >>" << e.what() << RESET << std::endl;
    }

}