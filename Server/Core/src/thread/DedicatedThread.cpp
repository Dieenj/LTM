#include "../../include/thread_manager.h"
#include "../../include/db_manager.h"
#include "../../include/thread_monitor.h"
#include "../../include/server_config.h"
#include "../../../../Common/Protocol.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE ServerConfig::BUFFER_SIZE
#define STORAGE_PATH ServerConfig::STORAGE_PATH

void DedicatedThread::handleUpload(int socketFd, std::string filename, long filesize, std::string username, long long parent_id, WorkerThread* workerRef) {
    ThreadMonitor::getInstance().reportDedicatedThreadStart();

    std::string path = std::string(STORAGE_PATH) + filename;
    std::ofstream outfile(path, std::ios::binary);
    
    if (!outfile.is_open()) {
        std::string err = std::string(CODE_FAIL) + " Cannot create file on server\n";
        send(socketFd, err.c_str(), err.length(), 0);
        close(socketFd);
        return;
    }
    
    std::string msg = std::string(CODE_DATA_OPEN) + " Ready to receive data\n";
    send(socketFd, msg.c_str(), msg.length(), 0);

    char buffer[BUFFER_SIZE];
    long totalReceived = 0;
    const long ACK_GROUP_SIZE = 1048576;
    long bytesSinceLastAck = 0;
    
    while (totalReceived < filesize) {
        int bytesRead = read(socketFd, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) break;

        outfile.write(buffer, bytesRead);
        totalReceived += bytesRead;
        bytesSinceLastAck += bytesRead;
        
        if (bytesSinceLastAck >= ACK_GROUP_SIZE && totalReceived < filesize) {
            std::string ack = std::string(CODE_CHUNK_ACK) + " Received " + std::to_string(totalReceived) + " bytes\n";
            send(socketFd, ack.c_str(), ack.length(), 0);
            bytesSinceLastAck = 0;
        }
    }

    outfile.close();

    bool saved = DBManager::getInstance().addFile(filename, totalReceived, username, parent_id);
    if (!saved) {
        std::cerr << "[Dedicated] Failed to save file metadata to database" << std::endl;
    }

    msg = std::string(CODE_TRANSFER_COMPLETE) + " Upload success\n";
    send(socketFd, msg.c_str(), msg.length(), 0);
    
    ThreadMonitor::getInstance().reportBytesTransferred(filesize);
    ThreadMonitor::getInstance().reportDedicatedThreadEnd();
    
    if (workerRef) {
        ClientSession restoredSession;
        restoredSession.socketFd = socketFd;
        restoredSession.username = username;
        restoredSession.isAuthenticated = true;
        
        workerRef->addClient(socketFd, restoredSession);
        std::cout << "[Dedicated] Socket " << socketFd << " returned with session (user: " << username << ")" << std::endl;
    }
}

void DedicatedThread::handleDownload(int socketFd, std::string filename, std::string username, WorkerThread* workerRef) {
    ThreadMonitor::getInstance().reportDedicatedThreadStart();

    std::string path = std::string(STORAGE_PATH) + filename;

    std::ifstream infile(path, std::ios::binary | std::ios::ate);
    if (!infile.is_open()) {
        std::string err = std::string(CODE_FAIL) + " File not found on server\n";
        send(socketFd, err.c_str(), err.length(), 0);
        
        if (workerRef) {
            ClientSession restoredSession;
            restoredSession.socketFd = socketFd;
            restoredSession.username = username;
            restoredSession.isAuthenticated = true;
            workerRef->addClient(socketFd, restoredSession);
        }
        ThreadMonitor::getInstance().reportDedicatedThreadEnd();
        return;
    }

    long filesize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    std::string msg = std::string(CODE_DATA_OPEN) + " " + std::to_string(filesize) + "\n";
    send(socketFd, msg.c_str(), msg.length(), 0);

    char buffer[BUFFER_SIZE];
    long totalSent = 0;
    const long ACK_GROUP_SIZE = 1048576;
    long bytesSinceLastAck = 0;
    
    while (!infile.eof()) {
        infile.read(buffer, BUFFER_SIZE);
        int bytesRead = infile.gcount();
        if (bytesRead > 0) {
            send(socketFd, buffer, bytesRead, 0);
            totalSent += bytesRead;
            bytesSinceLastAck += bytesRead;
            
            if (bytesSinceLastAck >= ACK_GROUP_SIZE && totalSent < filesize) {
                char ackBuf[256];
                fd_set readfds;
                struct timeval tv;
                FD_ZERO(&readfds);
                FD_SET(socketFd, &readfds);
                tv.tv_sec = 3;
                tv.tv_usec = 0;
                
                int ret = select(socketFd + 1, &readfds, NULL, NULL, &tv);
                if (ret > 0) {
                    int n = recv(socketFd, ackBuf, sizeof(ackBuf) - 1, 0);
                    if (n > 0) {
                        ackBuf[n] = '\0';
                    }
                }
                bytesSinceLastAck = 0;
            }
        }
    }
    
    infile.close();

    msg = std::string(CODE_TRANSFER_COMPLETE) + " Download success\n";
    send(socketFd, msg.c_str(), msg.length(), 0);

    ThreadMonitor::getInstance().reportBytesTransferred(filesize);
    ThreadMonitor::getInstance().reportDedicatedThreadEnd();
    
    if (workerRef) {
        ClientSession restoredSession;
        restoredSession.socketFd = socketFd;
        restoredSession.username = username;
        restoredSession.isAuthenticated = true;
        
        workerRef->addClient(socketFd, restoredSession);
        std::cout << "[Dedicated] Socket " << socketFd << " returned with session (user: " << username << ")" << std::endl;
    }
}