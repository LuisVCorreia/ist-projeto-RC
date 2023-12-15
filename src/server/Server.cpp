#include "AS.hpp"
#include <signal.h>

AS* globalASInstance = nullptr;

void sigintHandler(int signum) {
    std::cout << std::endl;
    if (globalASInstance != nullptr) {
        globalASInstance->handleSigint();
    }
    exit(signum);
}

int main(int argc, char *argv[]) {
    bool verbose = false;
    const char* port;
    if (argc == 1) {
        // use default port 
        port = DEFAULT_PORT;
    } else if (argc == 2 && strcmp(argv[1], "-v") == 0){
        // use default port verbose mode activated 
        port = DEFAULT_PORT;
        verbose = true;
    } else if (argc == 3 && strcmp(argv[1], "-p") == 0 && 
        std::all_of(argv[2], argv[2] + strlen(argv[2]), ::isdigit)) {
        // use specified port
        port = argv[2];
    }  else if (argc == 4 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-v") == 0) {
        // use specified port and verbose mode activated
        port = argv[2];
        verbose = true;
    } else {
        // invalid arguments
        std::cerr << "Usage: " << argv[0] << " [-p ASport] [-v]" << std::endl;
        return 1;
    }
    struct sigaction act;
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;

    AS as(port);
    globalASInstance = &as;

    memset(&act, 0, sizeof(act));
    act.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    as.run();

    return 0;
}