/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 13:20:37 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/20 10:03:57 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// # define DEBUG          1

#include "include/webserv.hpp"
// #include "include/Colors.hpp"
// #include "Exceptions.hpp"
#include "Worker.hpp"
#include "Config.hpp"
// #include "Parsing_config_file.hpp"


int main(int ac, char** av){
    (void)ac;
    try
    {
        Config  config(av[1]);
        
        COUT_DEBUG_INSERTION("Welcome to mini-serv" << std::endl);
        //* parsing configuration file
        config.parse_config();
        // //* initializing worker
        Worker worker(config.getConf());
        
        // //* starting worker
        worker.workerLoop();
    }
	catch (std::exception& e)
    {
		std::cout << std::endl << BOLDRED "! Caught exception >> " << e.what() << RESET << std::endl;
    }
    return (1);
}