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

#include "ConnectionSocket.hpp"

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
void handle_new_connection(int & server_fd, fd_set & fds, fd_set & fd_read, int & fd_max);
void handle_read_request(int server_fd, fd_set & fds, fd_set & fd_read, int & fd_max);

std::vector<ConnectionSocket>	clients;

int main (){ (void)clients;

    int                     		server_fd;
    int                     		fd_max;
    fd_set                  		fds, fd_read, fd_write;

    
    std::cout << "Welcome to mini-serv" << std::endl;
    create_server_socket(server_fd);
    make_non_blocking(server_fd);

    FD_ZERO(&fds);
    FD_SET(server_fd, &fds);
    fd_max = server_fd;

    while(true){
        fd_read = fds;
        fd_write = fds;

        //! select
        if (-1 == select(fd_max + 1, &fd_read, &fd_write, NULL, NULL)){
            close(server_fd);
            ftError("accept() failed");
        }

        //! handdle new connection
        handle_new_connection(server_fd, fds, fd_read, fd_max);
        //! handle read request

        //! 6.Receive communication
        handle_read_request(server_fd, fds, fd_read, fd_max);
    }
    //! 8.Closing connection
    std::cout << "Closing connection" << std::endl;
    close(server_fd);
    std::cout << "Exiting successfully mini-serv" << std::endl;
    return 0;
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
        close(server_fd);
        ftError("socket() failed");
    }
    //! 4.Listen
    std::cout << "---------------listen()" << std::endl;
    if (-1 == listen(server_fd, 3)){
        close(server_fd);
        ftError("listen() failed");
    }
}

void handle_new_connection(int & server_fd, fd_set & fds, fd_set & fd_read, int & fd_max)
{
    int                     cli_socket;
    struct sockaddr_in      cli_addr;
    socklen_t               cli_addr_len = sizeof(cli_addr);
   
    //* 1. checks if server_fd is set into read_fds
    if (FD_ISSET(server_fd, &fd_read)){
        //* 2. if so, accept and add new connection socket into fds, updating max if necessary (carry current max with us)
        std::cout << "---------------accept()" << std::endl;
        cli_socket = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
        if (-1 == cli_socket){
            close(server_fd);
            ftError("accept() failed");
        }
		std::cout << "new connection socket : " << cli_socket << std::endl;
		clients.push_back(ConnectionSocket(cli_socket));
        if (cli_socket > fd_max)
            fd_max = cli_socket;
        FD_SET(cli_socket, &fds);        
    }
}

void handle_read_request(int server_fd, fd_set & fds, fd_set & fd_read, int & fd_max){
    char buffer[BUFFER_SIZE];

//*1.   loop thorugh 0 to fd_max
    for (int fd_i = 0; fd_i <= fd_max; fd_i++){
///*1.2    for each, check if it is set inside fd_read (we may need to skip server fd)
		// std::cout << "fd_i : " << fd_i << std::endl;
        if (FD_ISSET(fd_i, &fd_read) && fd_i != server_fd){
//* 1.3   if it is set, read into buffer
        	std::cout << "---------------recv() fd_i : " << fd_i << std::endl;
			memset(buffer, '\0', BUFFER_SIZE);
            ssize_t bytes_received = recv(fd_i, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0){
                std::cout << "Received message from client.";
                std::cout << "| bytes_received : " << bytes_received;
                std::cout << "| message_received : " << std::endl << buffer << std::endl;
            }
            // else if (bytes_received == 0){
            if (0 == bytes_received || '\n' == buffer[bytes_received - 1]){
                //! process request
				//! 7.Send communication
				// if (FD_ISSET(fd_i, fd_write)) {}
				ssize_t bytes_sent = send(fd_i, MSG_HTML, MSG_SIZE, 0);
				if (bytes_sent > 0){
                	std::cout << "Response sent to the client.";
                	std::cout << "| bytes_sent : " << bytes_sent << std::endl;					
				}
				else if (-1 == bytes_sent){
					close(fd_i);
					FD_CLR(fd_i, &fds);
					ftError("send() failed");
				}
                //! process end of request
                std::cout << "Received message from client." << std::endl;
                std::cout << "Reached EOF OR client left" << std::endl;
                close(fd_i);
                FD_CLR(fd_i, &fds);
				//TODO	update fd_max !
            }
            else if (-1 == bytes_received){
                close(fd_i);
                FD_CLR(fd_i, &fds);
                ftError("recv() failed");
            }
        }
    }
//*1.4      if itś available for writing, echo back  
}







    //! retrieving sock info
    // if (getsockname(server_fd, (struct sockaddr *)&server_addr, &server_addr_len) == ERR) {
    //     ftError("getsockname() failed");
    // }
