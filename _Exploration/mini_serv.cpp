
#include <sys/socket.h>     // socket
#include <stdio.h>          // perror


int main (){

    // Creating socket file descriptor
    // int server_fd = socket(domain, type, protocol)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1){
        perror("socket_fd");
        exit(EXIT_FAILURE)
    }

    // helper in socket options
    // int setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen);
    if (setsockopt(server_fd, int level, int optname,  const void *optval, socklen_t optlen) == -1){
        perror("setsockopt");
        exit(EXIT_FAILURE)
    }

    
    // Bind
    int bind(server_fd, const struct sockaddr *addr, socklen_t addrlen);



    // Listen
    int listen(server_fd, int backlog);



    // Accept
    int new_socket= accept(int server_fd, struct sockaddr *addr, socklen_t *addrlen);



}