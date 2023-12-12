#include "serverTCP.hpp"

ServerTCP::ServerTCP(const char* port, int& tcp_socket) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    tcp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (tcp_socket == -1)
        exit(1); // TODO: Fix Error handling

    if (bind(tcp_socket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error TCP server");
        exit(1); // TODO: Fix Error handling
    }

    if (listen(tcp_socket, 5) == -1) {  // alterar este valor?
        perror("Listen error TCP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}