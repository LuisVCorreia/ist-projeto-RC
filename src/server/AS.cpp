#include "AS.hpp"

AS::AS(const char* port, int verbose)
    : serverUDP(port, socketUDP, verbose), serverTCP(port, socketTCP, verbose) {
}


void AS::run() {

    std::thread tcp_thread(&ServerTCP::handleTCP, &serverTCP);
    std::thread udp_thread(&ServerUDP::handleUDP, &serverUDP);
    
    udp_thread.join();
    tcp_thread.join();

}


void AS::handleSigint() {
    logoutAllUsers();
}