/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 13:20:37 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/06 21:38:47 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# define DEBUG          1

#include "Colors.hpp"
#include "Exceptions.hpp"
#include "WorkerServer.hpp"


int main(){

    try
    {
        std::cout << "Welcome to mini-serv" << std::endl;
        WorkerServer server;

        server.serverLoop();
    }
	catch (std::exception& e){
		std::cerr << std::endl << BOLDRED ">>" << e.what() << RESET << std::endl;
    }

}