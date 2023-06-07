/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 13:20:37 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/07 17:39:00 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# define DEBUG          1

#include "Colors.hpp"
#include "Exceptions.hpp"
#include "Worker.hpp"


int main(){

    try
    {
        std::cout << "Welcome to mini-serv" << std::endl;
        Worker server;

        server.workerLoop();
    }
	catch (std::exception& e){
		std::cout << std::endl << BOLDRED "Exception >>" << e.what() << RESET << std::endl;
    }

}