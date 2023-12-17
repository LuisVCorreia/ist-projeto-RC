#ifndef AS_H
#define AS_H

#include "serverUDP.hpp"
#include "serverTCP.hpp"

// #include <sys/types.h> //
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <fcntl.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <string.h>
// #include <errno.h>
// #include <algorithm>
// #include <iostream> // TODO remove these ones

#define MYPORT "58047"
#define DEFAULT_PORT "58047"

class AS {
    public:
        AS(const char* port);
        void run();
        void handleSigint();

        int socketUDP;
        int socketTCP;
        

    private:
        ServerUDP serverUDP;
        ServerTCP serverTCP;
};


#endif