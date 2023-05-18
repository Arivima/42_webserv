//! MINI_SERVER EXAMPLE

#include <iostream>         // cin cout cerr
#include <stdio.h>          // perror
#include <stdlib.h>         // exit, EXIT_FAILURE
#include <cstring>          // memset
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, recv
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_addr
#include <unistd.h>         // close
#include <fcntl.h>          // fcntl

# define    ERR         (-1)
# define    PORT        8080
# define    BUFFER_SIZE 1024

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

void ftError(char * msg){
    perror(msg);
    std::cout << "Exiting mini-serv with failure" << std::endl;
    exit(EXIT_FAILURE);
}


int main (){

    int                     server_fd;
    struct sockaddr_in      server_addr;
    socklen_t               server_addr_len = sizeof(server_addr);
    
    std::cout << "Welcome to mini-serv" << std::endl;
    //! 1.Creating socket file descriptor
    std::cout << "---------------socket()" << std::endl;
    // int server_fd = socket(domain (AF_INET = internet IP4), type (SOCK_STREAM = TCP), protocol)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == ERR){
        ftError("socket() failed");
    }
    //! Non blocking socket
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == ERR){
        ftError("fcntl() failed");
    }
    flags |= O_NONBLOCK;
    if (fcntl(server_fd, F_SETFL, flags) == ERR){
        ftError("fcntl() failed");
    }

    //! 2.Set up server address
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local available address
    //! Printing the current address info
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Local fam\t: %d\n", server_addr.sin_family);
    printf("Local address\t: %s\n", ip);
    printf("Local port\t: %d\n", ntohs(server_addr.sin_port));
    //! 3.Bind 
    std::cout << "---------------bind()" << std::endl;
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if (bind(server_fd, (struct sockaddr *)&server_addr, server_addr_len) == ERR){
        close(server_fd);
        ftError("socket() failed");
    }
    //! 4.Listen
    std::cout << "---------------listen()" << std::endl;
    // int listen(int server_fd, int backlog);
    if (listen(server_fd, 3) == ERR){
        close(server_fd);
        ftError("listen() failed");
    }
    
    //! Loop in (while(true)) for more than 5 communications
    int cli_socket;
    for (int i = 5; i >= 0; i--){ 
        //! 5.Accept
        std::cout << "---------------accept()" << std::endl;
        // int cli_socket= accept(int server_fd, struct sockaddr *addr, socklen_t *addrlen);
        std::cout << "Waiting for connection with client" << std::endl;
        cli_socket = accept(server_fd, (struct sockaddr *)&server_addr, &server_addr_len);
        if (cli_socket == ERR){
            close(server_fd);
            ftError("accept() failed");
        }
        //! 6.Receive communication
        // ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(cli_socket, &buffer, sizeof(buffer), 0);
        if (bytes_received > 0){
            std::cout << "Received message from client." << std::endl;
            std::cout << "bytes_received : " << bytes_received << std::endl;
            std::cout << "message_received : " << std::endl << buffer << std::endl;
        }
        else if (bytes_received == 0){
            std::cout << "Received message from client." << std::endl;
            std::cout << "Reached EOF" << std::endl;
        }
        else if (bytes_received == ERR){
            close(cli_socket);
            close(server_fd);
            ftError("recv() failed");
        }
        std::cout << "End of message from client." << std::endl;
        //! 7.Send communication
        // ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        std::string message_sent("Hello from server\n");
        ssize_t bytes_sent = send(cli_socket, &message_sent, sizeof(message_sent), 0);
        if (bytes_sent > 0){
            std::cout << "Response sent to the client." << std::endl;
            std::cout << "bytes_sent : " << bytes_sent << std::endl;
            std::cout << "message_sent : " << std::endl << message_sent << std::endl;
        }
        else if (bytes_sent == ERR){
            close(cli_socket);
            close(server_fd);
            ftError("send() failed");
        }
    }
    //! 8.Closing connection
    std::cout << "Closing connection" << std::endl;
    close(cli_socket);
    close(server_fd);
    std::cout << "Exiting successfully mini-serv" << std::endl;
    return 0;
}

    //! make IP reusable quickly
    // int setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen);
    // if (setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen) == ERR){
    //     ftError("setsockopt");
    // }

    //! set up concurrency option

    //! retrieving sock info
    // if (getsockname(server_fd, (struct sockaddr *)&server_addr, &server_addr_len) == ERR) {
    //     ftError("getsockname() failed");
    // }

    // AFTER Printing the current address info
    // char ip[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    // printf("Local fam\t: %d\n", server_addr.sin_family);
    // printf("Local address\t: %s\n", ip);
    // printf("Local port\t: %d\n", ntohs(server_addr.sin_port));
