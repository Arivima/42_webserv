/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorkerServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:35:54 by avilla-m          #+#    #+#             */
/*   Updated: 2023/05/30 14:23:20 by avilla-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKERSERVER_HPP
#define WORKERSERVER_HPP

#include <iostream>         // cin cout cerr
#include <string>           // strings, c_str()
#include <cstring>          // memset


#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv, bind, socklen_t
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_ntop
#include <fcntl.h>          // fcntl

#include <sys/select.h>     // select, FT_ISSET, FT_CLR, etc

#include <vector>           // vector

#include <unistd.h>         // close
#include <stdlib.h>         // exit, EXIT_FAILURE

#include "Exceptions.hpp"
#include "ClientConnection.hpp"


# define    PORT        8080
# define    BUFFER_SIZE 1024

# define	MSG_HTML	\
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: 146\r\n\
Connection: close\r\n\
\r\n\
<!DOCTYPE html> \
<html> \
<head> \
  <title>Example HTTP Response</title> \
</head> \
<body> \
  <h1>Hello, World!</h1> \
  <p>This is an example HTTP response.</p> \
</body> \
</html>"
# define MSG_SIZE	(sizeof(MSG_HTML))

typedef struct s_server_fd_list{
        int     server;
        int     max;
        fd_set  all_set;
        fd_set  read_set;
        fd_set  write_set;
}               t_server_fd_list;

class WorkerServer{
    private:
        void _create_server_socket(){
            std::cout << "---------------socket()" << std::endl;
            this->fds.server = socket(AF_INET, SOCK_STREAM, 0);
            if (-1 == this->fds.server)
                throw SystemCallException("socket()");
        }

        void _set_socket_as_reusable(){
            int reuse = 1;
            if (-1 == setsockopt(this->fds.server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))){
                throw SystemCallException("setsockopt()");
            }
        }

        void _init_server_addr(){
            std::memset(&this->server_addr, 0, this->server_addr_len);
            this->server_addr.sin_family = AF_INET;
            this->server_addr.sin_port = htons(PORT);
            this->server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local available address
        }

        void _print_server_ip_info(){
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(this->server_addr.sin_addr), ip, INET_ADDRSTRLEN);
            std::cout << "Local fam\t: " << this->server_addr.sin_family << std::endl;
            std::cout << "Local address\t: " << ip << std::endl;
            std::cout << "Local port\t: " << ntohs(this->server_addr.sin_port) << std::endl;  
        }
        
        void _bind_server_socket_to_ip(){
            std::cout << "---------------bind()" << std::endl;
            if (-1 == bind(this->fds.server, (struct sockaddr *)&this->server_addr, this->server_addr_len))
                throw SystemCallException("bind()");
        }

        void _make_server_listening(){
            std::cout << "---------------listen()" << std::endl;
            if (-1 == listen(this->fds.server, 3))
                throw SystemCallException("bind()");
        }

        void _make_server_non_blocking(){
            int flags = fcntl(this->fds.server, F_GETFL, 0);
            if (-1 == flags)
                throw SystemCallException("fcntl()");
            flags |= O_NONBLOCK;
            if (-1 == fcntl(this->fds.server, F_SETFL, flags))
                throw SystemCallException("fcntl()");
        }

        void _init_io_multiplexing(){
            FD_ZERO(&this->fds.all_set);
            FD_SET(this->fds.server, &this->fds.all_set);
            this->fds.max= this->fds.server;
        }

        void _io_multiplexing_using_select(){
            fds.read_set = fds.all_set;
            fds.write_set = fds.all_set;
            if (-1 == select(this->fds.max + 1, &this->fds.read_set, &this->fds.write_set, NULL, NULL))
                throw SystemCallException("select()");
        }

        void _handle_new_connection(){
            std::cout << YELLOW << "! Handling new connection !" << RESET << std::endl;
            ClientConnection        newClient;

            newClient.handle_new_connection(this->fds);
            clients.push_back(&newClient);
            // debug : print list of updated fds_all_set
            // debug : print list of client sockets
            std::cout << "fds.max: " << fds.max << std::endl;
        }

        void _handle_read_request(){
            std::cout << YELLOW << "! Handling new read request !" << RESET << std::endl;
        }

        WorkerServer(const WorkerServer & cpy){}
        WorkerServer& operator=(const WorkerServer & cpy){}

        void _server_init(){
            _create_server_socket();
            _set_socket_as_reusable();
            _init_server_addr();
            _print_server_ip_info();
            _bind_server_socket_to_ip();
            _make_server_listening();
            _make_server_non_blocking();
            _init_io_multiplexing();
        }

    public:
        std::vector<ClientConnection *> clients;
        t_server_fd_list                fds;
        struct sockaddr_in              server_addr;
        socklen_t                       server_addr_len = sizeof(server_addr);
        
        WorkerServer() : clients(), fds(), server_addr(), server_addr_len() {
            this->_server_init();
        }

        ~WorkerServer(){
            // delete each element of vector + clear the whole vector
            // this->clients.clear();
            
            std::cout << "Closing connection | worker: " << this->fds.server << std::endl;
            close(this->fds.server);
        }

        void serverLoop(){
            while (true){
                _io_multiplexing_using_select();
                _handle_new_connection();
                // _handle_read_request();
            }
        }
};


#endif

// void handle_read_request(int server_fd, fd_set & fds, fd_set & fd_read, int & fd_max){
//     char buffer[BUFFER_SIZE];

// //*1.   loop thorugh 0 to fd_max
//     for (int fd_i = 0; fd_i <= fd_max; fd_i++){
// ///*1.2    for each, check if it is set inside fd_read (we may need to skip server fd)
// 		// std::cout << "fd_i : " << fd_i << std::endl;
//         if (FD_ISSET(fd_i, &fd_read) && fd_i != server_fd){
// //* 1.3   if it is set, read into buffer
//         	std::cout << "---------------recv() fd_i : " << fd_i << std::endl;
// 			memset(buffer, '\0', BUFFER_SIZE);
//             ssize_t bytes_received = recv(fd_i, buffer, BUFFER_SIZE, 0);
//             if (bytes_received > 0){
//                 std::cout << "Received message from client.";
//                 std::cout << "| bytes_received : " << bytes_received;
//                 std::cout << "| message_received : " << std::endl << buffer << std::endl;
//             }
//             // else if (bytes_received == 0){
//             if (0 == bytes_received || '\n' == buffer[bytes_received - 1]){
//                 //! process request
// 				//! 7.Send communication
// 				// if (FD_ISSET(fd_i, fd_write)) {}
// 				ssize_t bytes_sent = send(fd_i, MSG_HTML, MSG_SIZE, 0);
// 				if (bytes_sent > 0){
//                 	std::cout << "Response sent to the client.";
//                 	std::cout << "| bytes_sent : " << bytes_sent << std::endl;					
// 				}
// 				else if (-1 == bytes_sent){
// 					close(fd_i);
// 					FD_CLR(fd_i, &fds);
// 					ftError("send() failed");
// 				}
//                 //! process end of request
//                 std::cout << "Received message from client." << std::endl;
//                 std::cout << "Reached EOF OR client left" << std::endl;
//                 close(fd_i);
//                 FD_CLR(fd_i, &fds);
// 				//TODO	update fd_max !
//             }
//             else if (-1 == bytes_received){
//                 close(fd_i);
//                 FD_CLR(fd_i, &fds);
//                 ftError("recv() failed");
//             }
//         }
//     }