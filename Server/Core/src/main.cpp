#include "../include/thread_manager.h"
#include "../include/db_manager.h"
#include "../include/thread_monitor.h"
#include "../include/server_config.h"
#include <iostream>
#include <thread>
#include <csignal>

AcceptorThread* globalAcceptor = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[Main] Received signal " << signum << ", shutting down..." << std::endl;
    if (globalAcceptor) {
        globalAcceptor->stop();
    }
    ThreadMonitor::getInstance().stop();
    exit(signum);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    ThreadMonitor::getInstance().start();
    std::cout << "[Main] ThreadMonitor started" << std::endl;
    
    if (!DBManager::getInstance().connect()) return -1;

    AcceptorThread acceptor(ServerConfig::SERVER_PORT);
    globalAcceptor = &acceptor;
    
    std::cout << "[Main] Server started with:" << std::endl;
    std::cout << "  - Port: " << ServerConfig::SERVER_PORT << std::endl;
    std::cout << "  - 1 AcceptorThread (Main)" << std::endl;
    std::cout << "  - Fixed Worker Pool (" 
              << ServerConfig::FIXED_WORKER_THREADS << " threads, load-balanced)" << std::endl;
    std::cout << "  - 1 MonitorThread (stats reporting)" << std::endl;
    std::cout << "  - DedicatedThreads (created on-demand for file I/O, max " 
              << ServerConfig::MAX_DEDICATED_THREADS << ")" << std::endl;
    
    acceptor.run(); // Hàm này có vòng lặp vô hạn accept()

    return 0;
}