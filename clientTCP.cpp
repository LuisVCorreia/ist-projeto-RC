#include "clientTCP.hpp"

ClientTCP::ClientTCP(const char* port, const char* asip) {
    struct addrinfo hints;
    int errcode;
    this->asip = asip; 
    struct sigaction act;
    act.sa_handler=SIG_IGN;
  
    if (sigaction (SIGPIPE,&act, NULL)== -1) /*error*/ exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(asip, port, &hints, &res);
    if (errcode != 0) exit(1);

}

ClientTCP::~ClientTCP() {
    freeaddrinfo(res);
    close(fd);
}

void ClientTCP::handleTest() {
    char buffer[129];

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) exit(1); // TODO remove all exits and handle errors
    
    int errcode = connect(fd, res->ai_addr, res->ai_addrlen);
    if (errcode == -1) exit(1); //TODO remove all exits and handle errors

    std::cout << "CONNECTED." << std::endl;

    ssize_t n = write(fd,"SAS 001\n", 8); 
    if(n==-1)/*error*/exit(1);

    std::cout << "WRITE: "  << n << std::endl;
 
    n=read(fd,buffer,128); 
    if(n==-1)/*error*/exit(1);

    std::cout << "READ: "  << n << std::endl;

    buffer[n] = '\0';

    std::cout << buffer << std::endl;

    // close connection

    close(fd);
}