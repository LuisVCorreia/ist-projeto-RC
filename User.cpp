#include "clientUDP.hpp"

// ./AS -n 193.136.138.142 -p 58011

int main(int argc, char *argv[]) {
    const char* port;
    const char* asip;
    
    if (argc == 1) {
        // use default port and default asip
        port = DEFAULT_PORT;
        asip = DEFAULT_ASIP; // TODO check this
    } else if (argc == 3 && strcmp(argv[1], "-n") == 0){
        // specify IP address of machine where AS runs
        port = DEFAULT_PORT;
        asip = argv[2];
    } else if (argc == 3 && strcmp(argv[1], "-p") == 0 && 
        std::all_of(argv[2], argv[2] + strlen(argv[2]), ::isdigit)) {
        // use specified port
        port = argv[2];
        asip = DEFAULT_ASIP;
    }  else if (argc == 5 && strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
        // use specified port and specified asip
        asip = argv[2];
        port = argv[4];
    } else {
        // invalid arguments
        std::cerr << "Usage: " << argv[0] << " [-n ASIP] [-p ASport]" << std::endl;
        return 1;
    }

    //std::cout << "port: " << port << std::endl;
    //std::cout << "asip: " << asip << std::endl;

    ClientUDP client(port, asip);
    client.run();


    return 0;
}