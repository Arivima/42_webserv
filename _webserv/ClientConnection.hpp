/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 11:29:56 by avilla-m          #+#    #+#             */
/*   Updated: 2023/05/31 10:17:58 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// ARIELLE WIP VERSION

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv, socklen_t
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <iostream>         // cin cout cerr
#include <unistd.h>         // close
#include <sys/select.h>     // sleect, FT_ISSET, FT_CLR, etc


#include "Exceptions.hpp"
#include "WorkerServer.hpp"


class ClientConnection{
    private:
        int                     _client_socket;

        ClientConnection(const ClientConnection & cpy){}
        ClientConnection &  operator=(const ClientConnection & cpy){}
    public:
        struct sockaddr_in      cli_addr;
        socklen_t               cli_addr_len = sizeof(cli_addr);

        ClientConnection() : _client_socket(-1) {}

        ~ClientConnection(){
            std::cout << "Closing connection | client: " << getClientSocket() << std::endl;
            close(getClientSocket());
        }

        int     getClientSocket(){return this->_client_socket;}
        void    setClientSocket(const int & newSocket){this->_client_socket = newSocket;}

        void    handle_new_connection(t_server_fd_list & fds){
            //* 2. if so, accept and add new connection socket into fds, updating max if necessary (carry current max with us)
            std::cout << "---------------accept()" << std::endl;
            setClientSocket(accept(fds.server, (struct sockaddr *)&cli_addr, &cli_addr_len));
            std::cout << "new connection socket : " << getClientSocket() << std::endl;
            if (-1 == getClientSocket())
                throw SystemCallException("accept()");
            if (getClientSocket() > fds.max)
                fds.max = getClientSocket();
            FD_SET(getClientSocket(), &fds.all_set);
        }
};


#endif //CLIENTCONNECTION_HPP