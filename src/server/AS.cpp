#include "AS.hpp"


AS::AS(const char* port)
    : serverUDP(port, socketUDP), serverTCP(port, socketTCP) {
}

void AS::run() {
    int out_fds;
    fd_set inputs, testfds;

    // Initialize fd_set
    FD_ZERO(&inputs);
    FD_SET(socketUDP, &inputs);
    FD_SET(socketTCP, &inputs);

    while (!session_terminated) {
        testfds = inputs;

        out_fds = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

        switch (out_fds) {
            case -1:
                perror("Select error");
                exit(1);
            case 0:
                // Timeout occurred
                break;
            default:
                if (FD_ISSET(socketUDP, &testfds)) {
                    AS::handleUDPRequest(socketUDP);
                }
                if (FD_ISSET(socketTCP, &testfds)) {
                    AS::handleTCPConnection(socketTCP);
                }
                break;
        }

    }

    close(socketUDP);
    close(socketTCP);
}


void AS::handleUDPRequest(int udp_socket) {
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Receive data from the UDP socket
    ssize_t received_bytes = recvfrom(udp_socket, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addrlen);
    if (received_bytes < 0) {
        perror("Error receiving UDP data");
        return;
    }

    buffer[received_bytes] = '\0';

    printf("Received UDP data: %s\n", buffer);

    // Add additional logic to handle the request as needed
    // ...
}

void AS::handleTCPConnection(int tcp_socket) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Accept the incoming TCP connection
    int new_sock = accept(tcp_socket, (struct sockaddr *)&client_addr, &addrlen);
    if (new_sock < 0) {
        perror("Accept error");
        return;
    }

    // Add additional logic to handle the request


    close(new_sock);
}

