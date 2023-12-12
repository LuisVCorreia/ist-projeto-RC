#include "serverUDP.hpp"

ServerUDP::ServerUDP(const char* port, int& udp_socket) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    udp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (udp_socket == -1)
        exit(1); // TODO: Fix Error handling

    if (bind(udp_socket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error UDP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}
