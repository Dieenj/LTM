#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <algorithm>
#include "server.h"

// Class xử lý đa nhiệm (Worker)
class WorkerThread {
public:
    WorkerThread();
    void addClient(int socketFd);
    void addClient(int socketFd, const ClientSession& session);  // Khôi phục session
    void run(); 
    void stop();

private:
    void handleClientMessage(int fd);
    
    // CẬP NHẬT: Thêm tham số closeSocket (mặc định là true)
    // Nếu false: Chỉ ngừng theo dõi, không đóng socket (để chuyển cho thread khác)
    void removeClient(int fd, bool closeSocket = true);

    std::vector<int> client_sockets;
    std::map<int, ClientSession> sessions;
    std::mutex mtx;
    std::atomic<bool> running;
    int epoll_fd;  // epoll file descriptor
};

// Class xử lý riêng (Dedicated)
class DedicatedThread {
public:
    void handleUpload(int socketFd, std::string filename, long filesize, std::string username, WorkerThread* workerRef);
    void handleDownload(int socketFd, std::string filename, std::string username, WorkerThread* workerRef);
};

// Class chấp nhận kết nối (Acceptor)
class AcceptorThread {
public:
    AcceptorThread(int port, WorkerThread& worker);
    void run();
private:
    int server_fd;
    int port;
    WorkerThread& workerRef;
};

#endif // THREAD_MANAGER_H