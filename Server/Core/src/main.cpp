#include "../include/thread_manager.h"
#include "../include/db_manager.h"
#include "../include/thread_monitor.h"
#include <iostream>
#include <thread>

int main() {
    // 0. Khởi động Thread Monitor
    ThreadMonitor::getInstance().start();
    std::cout << "[Main] ThreadMonitor started" << std::endl;
    
    // 1. Kết nối Database
    if (!DBManager::getInstance().connect()) return -1;

    // 2. Khởi tạo Worker Thread (Xử lý tác vụ nhẹ)
    WorkerThread worker;
    std::thread worker_t([&worker](){
        worker.run();
    });
    
    // Đăng ký worker với Monitor
    ThreadMonitor::getInstance().registerWorkerThread(&worker, worker_t.get_id());
    worker_t.detach();

    // 3. Khởi tạo Acceptor Thread (Lắng nghe cổng 8080)
    AcceptorThread acceptor(8080, worker);
    
    std::cout << "[Main] Server started with:" << std::endl;
    std::cout << "  - 1 AcceptorThread (Main)" << std::endl;
    std::cout << "  - 1 WorkerThread (epoll-based, handles 10-50 connections)" << std::endl;
    std::cout << "  - 1 MonitorThread (auto-scaling enabled)" << std::endl;
    std::cout << "  - DedicatedThreads (created on-demand for file I/O)" << std::endl;
    
    acceptor.run(); // Hàm này có vòng lặp vô hạn accept()

    return 0;
}