#include "serverTCP.hpp"

ServerTCP::ServerTCP(const char* port, int& socketTCP) {
    // Create the TCP socket

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
        exit(1); // TODO: Fix Error handling

    socketTCP = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketTCP == -1)
        exit(1); // TODO: Fix Error handling

    if (bind(socketTCP, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Bind error TCP server");
        exit(1); // TODO: Fix Error handling
    }

    if (listen(socketTCP, 5) == -1) {  // alterar este valor?
        perror("Listen error TCP server");
        exit(1); // TODO: Fix Error handling
    }
    freeaddrinfo(res);

}


void ServerTCP::receiveRequest(int& socketTCP){
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Accept the incoming TCP connection
    int new_sock = accept(socketTCP, (struct sockaddr *)&client_addr, &addrlen);
    if (new_sock < 0) {
        perror("Accept error");
        return;
    }

    struct timeval tv;
    tv.tv_sec = 1; // FIX ME: check this?
    tv.tv_usec = 0;
    setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int n;
    char buffer[1024];
    std::string request;
    
    while ((n = read(new_sock, buffer, sizeof(buffer) - 1)) > 0 ) {
        // Null-terminate the received data
        buffer[n] = '\0';

        // Append the received data to the string
        request += buffer;

        if (buffer[n-1] == '\n') break; // message complete
    }

    // send ERR if error during read operation
    if (n==-1){
        if (write(new_sock, "ERR\n", 4) == -1) 
            std::cout << "WARNING: write error\n";
        close(new_sock);
        return;
    }

    printf("Received TCP data: %s\n", request.c_str());

    // Add additional logic to handle the request


    close(new_sock);
}



