/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorkerServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 12:35:54 by avilla-m          #+#    #+#             */
/*   Updated: 2023/06/06 20:58:47 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKERSERVER_HPP
#define WORKERSERVER_HPP

// INCLUDES
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

#include "../include/webserv.hpp"
#include "../_Exploration/ConnectionSocket.hpp"
#include "../_Exploration/EpollData.hpp"
#include "Exceptions.hpp"

class WorkerServer{
private:
	int											server_fd;
    size_t										workerPort;
	t_epoll_data								edata;
	
public:
    typedef  std::vector<ConnectionSocket *>	VectorCli;

    std::vector<ConnectionSocket *>				clients;
    struct sockaddr_in             				server_addr;
    socklen_t                      				server_addr_len;

    private:
	void _create_server_socket(){
            std::cout << "---------------socket()" << std::endl;
            this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (-1 == this->server_fd)
                throw SystemCallException("socket()");
			this->workerPort = PORT_BASE_NUM + server_fd;
        }

	void _set_socket_as_reusable(){
			int reuse = 1;
			if (-1 == setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))){
				throw SystemCallException("setsockopt()");
			}
	}

	void _init_server_addr(){
		std::memset(&this->server_addr, 0, this->server_addr_len);
		this->server_addr.sin_family = AF_INET;
		this->server_addr.sin_port = htons(workerPort);
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
		if (-1 == bind(this->server_fd, (struct sockaddr *)&this->server_addr, this->server_addr_len))
			throw SystemCallException("bind()");
	}

	void _make_server_listening(){
		std::cout << "---------------listen()" << std::endl;
		if (-1 == listen(this->server_fd, 3))
			throw SystemCallException("listen()");
	}

	void _make_socket_non_blocking(int sock_fd){
		int flags = fcntl(sock_fd, F_GETFL, 0);
		if (-1 == flags)
			throw SystemCallException("fcntl()");
		flags |= O_NONBLOCK;
		if (-1 == fcntl(sock_fd, F_SETFL, flags))
			throw SystemCallException("fcntl()");
	}

	void _init_io_multiplexing(){
		struct epoll_event	eevent;

		this->edata.epoll_fd = epoll_create(1);
		if (-1 == this->edata.epoll_fd) {
			throw SystemCallException("epoll_create()");
		}
		eevent.events = EPOLLIN;
		eevent.data.fd = this->server_fd;
		if (-1 == epoll_ctl(this->edata.epoll_fd, EPOLL_CTL_ADD, this->server_fd, &eevent)) {
			throw SystemCallException("epoll_ctl()");
		}
	}

	void _io_multiplexing_using_epoll(){
		memset(this->edata.eeventS, 0, sizeof(struct epoll_event) * MAX_EVENTS);
		this->edata.n_events = epoll_wait(this->edata.epoll_fd, this->edata.eeventS, MAX_EVENTS, -1);
		if (-1 == this->edata.n_events) {
			throw SystemCallException("epoll_wait()");
		}
	}

	void _handle_new_connection() {
		int						cli_socket;
		struct sockaddr_in		cli_addr;
		socklen_t				cli_addr_len = sizeof(cli_addr);
		struct epoll_event		cli_epoll_evt_opt;

		if (this->edata.getEpollEvent(this->server_fd))
		{
    		//*     new socket creation
    		// std::cout << "handling new connection" << std::endl;
    	    std::cout  << std::endl << "\033[1m\033[32m""handle_new_connection() called()""\033[0m" << std::endl;
    		std::cout << "---------------accept()" << std::endl;
    		cli_socket = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    		if (-1 == cli_socket){
				throw (SystemCallException("accept()"));
    		}
    		_make_socket_non_blocking(cli_socket);
			std::cout << "new connection socket : " << cli_socket << std::endl;
				
    		//*     epoll
    		cli_epoll_evt_opt.events = EPOLLIN | EPOLLOUT;
    		cli_epoll_evt_opt.data.fd = cli_socket;
    		if (-1 == epoll_ctl(this->edata.epoll_fd, EPOLL_CTL_ADD, cli_socket, &cli_epoll_evt_opt)) {
				close(cli_socket);
				throw (SystemCallException("epoll_ctl()"));
    		}
					
    	    //*     modifying rcv buffer
    	    // int buffer_size = 8001;
    	    // if (-1 == setsockopt(cli_socket, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)))
    	    // {
    	    //     ft_close(server_fd);
    	    //     ftError("setsockopt() failed : setting rcv buffer size");
    	    // }
						
    		//*     adding to list of clients
			this->clients.push_back(new ConnectionSocket(cli_socket, edata));
    	    std::cout << std::endl;
		}
	}

	void	_serve_client( ConnectionSocket& client ) {

		const struct epoll_event*	eevent = edata.getEpollEvent(client.getSockFD());

		if (ConnectionSocket::e_READ_MODE == client.getStatus())
		{
			client.parse_line();
    	}
    	else {
			if (0 == client.flag) {
				if (!eevent || !(eevent->events & EPOLLOUT))
					return ;
    	        std::cout << "end of request" << std::endl;
    	        client.print_req();
    	        std::cout << "response mode" << std::endl;
    	        ssize_t bytes_sent = send(client.getSockFD(), SIMPLE_HTML_RESP, SIMPLE_HTML_RESP_SIZE, 0);
    	        std::cout << "send done" << std::endl;
				if (bytes_sent > 0){
    	        	std::cout << "Response sent to the client.";
    	        	std::cout << "| bytes_sent : " << bytes_sent << std::endl;
				}
				else if (-1 == bytes_sent){
    	            std::cout << "HERE" << std::endl;
    	            close(client.getSockFD());//TODO		forse la metto dentro il distruttore del client
    	            // clit = clients.erase(clit);
				}
    	        else {
    	            std::cout << "Maremma li mortacci" << std::endl;
    	        }
    	        client.flag = 1;
			}
    	}
	}

	void _handle_read_request(){
		std::cout << YELLOW << "! Handling new read request !" << RESET << std::endl;
	}

	WorkerServer(const WorkerServer & cpy){
		*this = cpy;
	}
	WorkerServer& operator=(const WorkerServer & cpy){
		(void)cpy;

		throw (std::exception());
		return (*this);
	}

	void _server_init(){
		_create_server_socket();
		_set_socket_as_reusable();
		_init_server_addr();
		_print_server_ip_info();
		_bind_server_socket_to_ip();
		_make_server_listening();
		_make_socket_non_blocking(this->server_fd);
		_init_io_multiplexing();
	}
public:
	WorkerServer()
		: server_fd(), workerPort(),
		edata(), clients(), server_addr(), server_addr_len(sizeof(server_addr))
	{
		this->_server_init();
	}

	~WorkerServer(){
		// test
		for (VectorCli::iterator cli_it = this->clients.begin(); cli_it != this->clients.end(); cli_it++)
			this->clients.erase(cli_it);
		this->clients.clear();
		std::cout << "Closing connection | worker: " << this->server_fd << std::endl;
		close(this->server_fd);
	}

	void serverLoop(){
		std::vector<ConnectionSocket *>::iterator	it;
		
		while (true){
			try {
				_io_multiplexing_using_epoll();
				_handle_new_connection();
			}
			catch (const SystemCallException& e) {
				std::cout << e.what() << std::endl;
				if ("epoll_wait()" == e._sysCall)
					continue ;
			}
			for (it = this->clients.begin(); it != this->clients.end();) {
				try {
					_serve_client(*(*it));
					it++;
				}
				catch (const ConnectionSocket::SockEof& e) {
					std::cout << e.what() << std::endl;
					it = clients.erase(it);
				}
			}
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