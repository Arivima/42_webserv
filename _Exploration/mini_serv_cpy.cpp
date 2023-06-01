//! MINI_SERVER EXAMPLE

#include <vector>

#include <iostream>         // cin cout cerr
#include <stdio.h>          // perror
#include <stdlib.h>         // exit, EXIT_FAILURE
#include <cstring>          // memset
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_addr
#include <unistd.h>         // close
#include <fcntl.h>          // fcntl
#include <sys/epoll.h>      // epoll

#include "ConnectionSocket.hpp"

# define    PORT        8080
# define    BUFFER_SIZE 1024
# define    MAX_EVENTS 4096

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


//! STEPS
// 1. socket
// 2. set IP addr
// 3. bind
// 4. listen
// 5. accept
// 6. receive request from client
// 7. write response to client
// 8. close connection

//! Reminder
// struct sockaddr_in {
// sa_family_t    sin_family; /* address family: always AF_INET */
// in_port_t      sin_port;   /* port in network byte order */
// struct in_addr sin_addr;   /* IP host address - internet address */
// };
// /* Internet address */
// struct in_addr {
// uint32_t       s_addr;     /* host interface address - address in network byte order */
// }; in_addr should be assigned one of the INADDR_* values (e.g., INADDR_LOOPBACK) using htonl(3)

void ftError(std::string msg);
void reuse_socket(int & server_fd);
void make_non_blocking(int & server_fd);
void print_ip(struct sockaddr_in & server_addr);
void set_ip(struct sockaddr_in & server_addr);
void create_server_socket(int & server_fd);
void handle_new_connection(int & server_fd, int epoll_fd);// fd_set & fds, fd_set & fd_read, int & fd_max);
void handle_read_request(struct epoll_event& eevent);// fd_set & fds, fd_set & fd_read, int & fd_max);
int ft_close(int fd);
struct epoll_event* getEpollEvent(int sock_fd, struct epoll_event* eeventS, size_t n_events);

struct epoll_event* getEpollEvent(int sock_fd, struct epoll_event* eeventS, size_t n_events)
{
	size_t	i;

	for (i = 0; i < n_events; i++)
		if (sock_fd == eeventS[i].data.fd)
			break ;
	return (i == n_events ? NULL : &eeventS[i]);
}

std::vector<ConnectionSocket *>	clients;

int main (){ (void)clients;

    int                     					server_fd;
    // int                     					fd_max;
    // fd_set                  					fds, fd_read, fd_write;
    int                             			epoll_fd;
    struct epoll_event              			eeventS[MAX_EVENTS];
    struct epoll_event              			eevent;
    int                             			n_events;
	std::vector<ConnectionSocket *>::iterator	it;


    
    std::cout << "Welcome to mini-serv" << std::endl;
    create_server_socket(server_fd);
    make_non_blocking(server_fd);


    // FD_ZERO(&fds);
    // FD_SET(server_fd, &fds);
    // fd_max = server_fd;
    epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        close(server_fd);
        ftError("epoll_crate() failed");
    }
    eevent.events = EPOLLIN;
    eevent.data.fd = server_fd;
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &eevent)) {
        close(server_fd);
        ftError("epoll_ctl() failed");
    }

    while(true){


        // //! select
        // fd_read = fds;
        // fd_write = fds;
        // if (-1 == select(fd_max + 1, &fd_read, &fd_write, NULL, NULL)){
        //     ft_close(server_fd);
        //     ftError("accept() failed");
        // }
        memset(eeventS, 0, sizeof(struct epoll_event) * MAX_EVENTS);
        n_events = epoll_wait(epoll_fd, eeventS, MAX_EVENTS, -1);
        if (-1 == n_events) {
            close(server_fd);
            ftError("epoll_wait() failed()");
        }

		handle_new_connection(
			server_fd,
			epoll_fd,
			getEpollEvent(server_fd, eeventS, n_events)
			);
		for (it = clients.begin(); it != clients.end(); it++)
			serve_client(
				(*it),
				getEpollEvent((*it)->getSockFD(), eeventS, n_events)
				);
		
        // for (int i = 0; i < n_events; i++) {
        //     if (server_fd == eeventS[i].data.fd)
        //         //! handdle new connection
        //         handle_new_connection(server_fd, epoll_fd);// fds, fd_read, fd_max);
        //     else
        //         //! handle read request
        //         handle_read_request(eeventS[i]);// fds, fd_read, fd_max);
        // }

    }
    //! 8.Closing connection
    std::cout << "Closing connection" << std::endl;
    ft_close(server_fd);
    std::cout << "Exiting successfully mini-serv" << std::endl;
    return 0;
}

int ft_close(int fd) {
    std::cout << "Closing fd: " << fd << std::endl;
    return (close(fd));
}

void ftError(std::string msg){
    perror(msg.c_str());
    std::cout << "Exiting mini-serv with failure" << std::endl;
    exit(EXIT_FAILURE);
}

void reuse_socket(int & server_fd){
    int reuse = 1;
    if (-1 == setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))){
        ftError("setsockopt");
    }
}

void print_ip(struct sockaddr_in & server_addr){
    char ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    std::cout << "Local fam\t: " << server_addr.sin_family << std::endl;
    std::cout << "Local address\t: " << ip << std::endl;
    std::cout << "Local port\t: " << ntohs(server_addr.sin_port) << std::endl;  
}

void set_ip(struct sockaddr_in & server_addr){
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local available address
}

void make_non_blocking(int & server_fd){
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (-1 == flags){
        ftError("fcntl() failed");
    }
    flags |= O_NONBLOCK;
    if (-1 == fcntl(server_fd, F_SETFL, flags)){
        ftError("fcntl() failed");
    }
}

void create_server_socket(int & server_fd){
    struct sockaddr_in      server_addr;
    socklen_t               server_addr_len = sizeof(server_addr);

    //! 1.Creating socket file descriptor
    std::cout << "---------------socket()" << std::endl;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == server_fd){
        ftError("socket() failed");
    }
    //! Set up option to reuse quickly the socket address
    reuse_socket(server_fd);

    //! 2.Set up server address
    set_ip(server_addr);

    //! Printing the current address info
    print_ip(server_addr);

    //! 3.Bind 
    std::cout << "---------------bind()" << std::endl;
    if (-1 == bind(server_fd, (struct sockaddr *)&server_addr, server_addr_len)){
        ft_close(server_fd);
        ftError("socket() failed");
    }
    //! 4.Listen
    std::cout << "---------------listen()" << std::endl;
    if (-1 == listen(server_fd, 3)){
        ft_close(server_fd);
        ftError("listen() failed");
    }
}

void handle_new_connection(int & server_fd, int epoll_fd)
{
    int                     cli_socket;
    struct sockaddr_in      cli_addr;
    socklen_t               cli_addr_len = sizeof(cli_addr);
    struct epoll_event      eevent;

    //*     new socket creation
    std::cout << "handling new connection" << std::endl;
    std::cout << "---------------accept()" << std::endl;
    cli_socket = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if (-1 == cli_socket){
        ft_close(server_fd);
        ftError("accept() failed");
    }
    make_non_blocking(cli_socket);
	std::cout << "new connection socket : " << cli_socket << std::endl;

    //*     epoll
    eevent.events = EPOLLIN ;//| EPOLLOUT;
    eevent.data.fd = cli_socket;
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_socket, &eevent)) {
        ft_close(server_fd);
        ftError("epoll_ctl() failed");
    }
    
    //*     adding to list of clients
	clients.push_back(new ConnectionSocket(cli_socket));


    //* 1. checks if server_fd is set into read_fds
    // if (FD_ISSET(server_fd, &fd_read)){
    //     //* 2. if so, accept and add new connection socket into fds, updating max if necessary (carry current max with us)
    //     if (cli_socket > fd_max)
    //         fd_max = cli_socket;
    //     FD_SET(cli_socket, &fds);
    //     FD_CLR(server_fd, &fd_read);
    // }
}

void handle_read_request(struct epoll_event& eevent){//, fd_set & fds, fd_set & fd_read, int & fd_max){
// (void)server_fd; (void)fds; (void)fd_read; (void) fd_max;
    std::vector<ConnectionSocket *>::iterator   it;

    //* std::cout << "handling read req" << std::endl;
    it = clients.begin();
    while (true)
    {
        if (eevent.data.fd == (*it)->getSockFD())
            break ;
        it++;
    }

    try {
        (*it)->parse_line();
        //* std::cout << "parse line done" << std::endl;

        if (0 == (*it)->flag) {
            //TODO      check if send() sent all the data.
            //TODO      if not, implement a mechansim to send the rest.
		    ssize_t bytes_sent = send((*it)->getSockFD(), MSG_HTML, MSG_SIZE, 0);
            std::cout << "send done" << std::endl;
			if (bytes_sent > 0){
            	std::cout << "Response sent to the client.";
            	std::cout << "| bytes_sent : " << bytes_sent << std::endl;
			}
			else if (-1 == bytes_sent){
                std::cout << "HERE" << std::endl;
                ft_close((*it)->getSockFD());//TODO    forse la metto dentro il distruttore del client
                clients.erase(it);
			}
            else {
                std::cout << "Maremma li mortacci" << std::endl;
            }
            (*it)->flag = 1;
        }
    }
    catch (const std::exception& e) {
        std::cout << "li mortacci" << std::endl;
        std::cout << e.what() << std::endl;
        ft_close((*it)->getSockFD());//TODO    forse la metto dentro il distruttore del client
        clients.erase(it);
    }

    // {
        // std::cout << "client n°: " << (*it)->getSockFD() << std::endl;

        // if (FD_ISSET((*it)->getSockFD(), &fd_read)) {
        //     std::cout << "handling read req" << std::endl;
        //     try {
        //         (*it)->parse_line();
        //         std::cout << "parse line done" << std::endl;

        //         if (0 == (*it)->flag) {
		// 		    ssize_t bytes_sent = send((*it)->getSockFD(), MSG_HTML, MSG_SIZE, 0);
        //             std::cout << "send done" << std::endl;
		// 		    if (bytes_sent > 0){
        //             	std::cout << "Response sent to the client.";
        //             	std::cout << "| bytes_sent : " << bytes_sent << std::endl;					
		// 		    }
		// 		    else if (-1 == bytes_sent){
        //                 std::cout << "HERE" << std::endl;
        //                 ft_close((*it)->getSockFD());//TODO    forse la metto dentro il distruttore del client
        //                 FD_CLR((*it)->getSockFD(), &fds);
        //                 it = clients.erase(it);
		// 		    }
        //             else {
        //                 std::cout << "Maremma li mortacci" << std::endl;
        //             }
        //             (*it)->flag = 1;
        //         }
                
        //     }
        //     catch (const std::exception& e) {
        //         std::cout << "li mortacci" << std::endl;
        //         std::cout << e.what() << std::endl;
        //         ft_close((*it)->getSockFD());//TODO    forse la metto dentro il distruttore del client
        //         // FD_CLR((*it)->getSockFD(), &fds);
        //         it = clients.erase(it);
        //     }
        //         FD_CLR((*it)->getSockFD(), &fd_read);
        //         std::cout << std::endl;
        // }
    // }



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
// 					ft_close(fd_i);
// 					FD_CLR(fd_i, &fds);
// 					ftError("send() failed");
// 				}
//                 //! process end of request
//                 std::cout << "Received message from client." << std::endl;
//                 std::cout << "Reached EOF OR client left" << std::endl;
//                 ft_close(fd_i);
//                 FD_CLR(fd_i, &fds);
// 				//TODO	update fd_max !
//             }
//             else if (-1 == bytes_received){
//                 ft_close(fd_i);
//                 FD_CLR(fd_i, &fds);
//                 ftError("recv() failed");
//             }
//         }
//     }
//*1.4      if itś available for writing, echo back  
}







    //! retrieving sock info
    // if (getsockname(server_fd, (struct sockaddr *)&server_addr, &server_addr_len) == ERR) {
    //     ftError("getsockname() failed");
    // }
