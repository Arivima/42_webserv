//? MINI_SERVER EXAMPLE

#include <iostream>         // cin cout cerr
#include <stdio.h>          // perror
#include <unistd.h>         // close
#include <stdlib.h>         // exit, EXIT_FAILURE
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM, 
#include <netinet/in.h>     // sockaddr_in struct, INADDR_ANY
#include <arpa/inet.h>      // inet_addr

# define PORT 8080

// STEPS
// 1. socket
// 2. set IP addr
// 3. bind
// 4. listen
// 5. accept

// struct sockaddr_in {
// sa_family_t    sin_family; /* address family: always AF_INET */
// in_port_t      sin_port;   /* port in network byte order */
// struct in_addr sin_addr;   /* IP host address - internet address */
// };
// /* Internet address */
// struct in_addr {
// uint32_t       s_addr;     /* host interface address - address in network byte order */
// }; in_addr should be assigned one of the INADDR_* values (e.g., INADDR_LOOPBACK) using htonl(3)


int main (){

    int                     server_fd;
    struct sockaddr_in      server_addr;
    socklen_t               server_addr_len = sizeof(server_addr);
    
    std::cout << "Welcome to mini-serv" << std::endl;

    std::cout << "---------------socket()" << std::endl;
    // Creating socket file descriptor
    // int server_fd = socket(domain (AF_INET = internet IP4), type (SOCK_STREAM = TCP), protocol)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1){
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // // make IP reusable quickly
    // // int setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen);
    // if (setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen) == -1){
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local available address

    // Printing the current address info
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Local fam\t: %d\n", server_addr.sin_family);
    printf("Local address\t: %s\n", ip);
    printf("Local port\t: %d\n", ntohs(server_addr.sin_port));

    std::cout << "---------------bind()" << std::endl;
    // Bind 
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if (bind(server_fd, (struct sockaddr *)&server_addr, server_addr_len) == -1){
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "---------------listen()" << std::endl;
    // Listen
    // int listen(int server_fd, int backlog);
    if (listen(server_fd, 3) == -1){
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    // make non blocking
    // set up concurrency option

    std::cout << "---------------accept()" << std::endl;
    // Accept
    // int new_socket= accept(int server_fd, struct sockaddr *addr, socklen_t *addrlen);
    int new_socket = accept(server_fd, (struct sockaddr *)&server_addr, &server_addr_len);
    if (new_socket == -1){
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }

    // ICI

    close(new_socket);
    close(server_fd);

    return 0;
}


    // retrieving sock info
    // if (getsockname(server_fd, (struct sockaddr *)&server_addr, &server_addr_len) == -1) {
    //     perror("getsockname() failed");
    //     exit(EXIT_FAILURE);
    // }

    // AFTER Printing the current address info
    // char ip[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &(server_addr.sin_addr), ip, INET_ADDRSTRLEN);
    // printf("Local fam\t: %d\n", server_addr.sin_family);
    // printf("Local address\t: %s\n", ip);
    // printf("Local port\t: %d\n", ntohs(server_addr.sin_port));
