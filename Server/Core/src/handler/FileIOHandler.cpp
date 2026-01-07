#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include "../../include/server_config.h"
#include "../../../../Common/Protocol.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

std::string FileIOHandler::handleQuotaCheck(const ClientSession& session, long filesize) {
    if (!session.isAuthenticated) {
        return std::string(CODE_LOGIN_FAIL) + " Not logged in\n";
    }
    
    std::string userPath = std::string(ServerConfig::STORAGE_PATH);
    if (!userPath.empty() && userPath.back() == '/') {
        userPath.pop_back();
    }
    
    if (!fs::exists(userPath)) {
        try {
            fs::create_directories(userPath);
        } catch (...) {
            return std::string(CODE_FAIL) + " Server Storage Error\n";
        }
    }

    long used = DBManager::getInstance().getStorageUsed(session.username);
    long limit = 1073741824;
    
    if (used + filesize > limit) {
        return std::string(CODE_FAIL) + " Quota exceeded\n";
    }
    
    return std::string(CODE_OK) + " Quota OK\n";
}

bool FileIOHandler::checkDownloadPermission(const ClientSession& session, const std::string& filename) {
    if (!session.isAuthenticated) {
        return false;
    }
    
    std::string filePath = std::string(ServerConfig::STORAGE_PATH) + filename;
    
    if (!fs::exists(filePath)) {
        std::cerr << "[FileIOHandler] File not found: " << filePath << std::endl;
        return false;
    }
    
    DBManager& dbManager = DBManager::getInstance();
    if (!dbManager.connect()) {
        std::cerr << "[FileIOHandler] Database connection failed!" << std::endl;
        return false;
    }
    
    std::string owner = dbManager.getFileOwner(filename);
    
    if (owner.empty()) {
        return true;
    }
    
    if (owner == session.username) {
        return true;
    }
    
    bool isShared = dbManager.isFileSharedWithUser(filename, session.username);
    
    if (isShared) {
        std::cout << "[FileIOHandler] File '" << filename << "' is shared with user '" << session.username << "'" << std::endl;
        return true;
    }
    
    std::cerr << "[FileIOHandler] Permission denied for user '" << session.username << "' to access '" << filename << "'" << std::endl;
    return false;
}