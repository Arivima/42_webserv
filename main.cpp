/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 13:20:37 by avilla-m          #+#    #+#             */
/*   Updated: 2023/05/30 14:17:08 by avilla-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# define DEBUG          1

#include "Colors.hpp"
#include "Exceptions.hpp"
#include "ClientConnection.hpp"
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