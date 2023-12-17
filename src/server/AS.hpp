#ifndef AS_H
#define AS_H

#include "serverUDP.hpp"
#include "serverTCP.hpp"

#define MYPORT "58047"
#define DEFAULT_PORT "58047"

class AS {
    public:
        AS(const char* port, int verbose);
        void run();
        void handleSigint();

        int socketUDP;
        int socketTCP;
        

    private:
        ServerUDP serverUDP;
        ServerTCP serverTCP;
};


#endif