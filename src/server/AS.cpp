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
                    serverUDP.receiveRequest();
                }
                if (FD_ISSET(socketTCP, &testfds)) {
                    serverTCP.receiveRequest();
                }
                break;
        }

    }

    close(socketUDP);
    close(socketTCP);
}


