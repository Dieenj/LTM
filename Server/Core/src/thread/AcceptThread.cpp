#include "../../include/thread_manager.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

AcceptorThread::AcceptorThread(int p, WorkerThread& w) : port(p), workerRef(w) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Cấu hình để tái sử dụng cổng ngay khi tắt server (tránh lỗi Address already in use)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

void AcceptorThread::run() {
    std::cout << "[Acceptor] Listening on port " << port << "..." << std::endl;
    
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (true) {
        // Chấp nhận kết nối từ Client
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept error");
            continue;
        }

        // Lấy địa chỉ IP của client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(address.sin_port);
        
        std::cout << "[Acceptor] New connection from " << client_ip 
                  << ":" << client_port << " (FD: " << new_socket << ")" << std::endl;
        
        // Chuyển ngay socket này sang WorkerThread để xử lý
        workerRef.addClient(new_socket);
    }
}