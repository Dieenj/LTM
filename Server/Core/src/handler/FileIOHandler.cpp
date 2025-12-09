#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include "../../../../Common/Protocol.h"
#include <iostream>

std::string FileIOHandler::handleQuotaCheck(const ClientSession& session, long filesize) {
    std::cout << "[QuotaCheck] User: " << session.username 
              << ", Auth: " << session.isAuthenticated 
              << ", FileSize: " << filesize << std::endl;
    
    // Kiểm tra đã đăng nhập chưa
    if (!session.isAuthenticated) {
        std::cout << "[QuotaCheck] FAILED: Not logged in" << std::endl;
        return std::string(CODE_LOGIN_FAIL) + " Not logged in\n";
    }
    
    // Lấy storage limit và used từ database
    long used = DBManager::getInstance().getStorageUsed(session.username);
    long limit = 1073741824; // TODO: Lấy từ USERS table
    
    std::cout << "[QuotaCheck] Used: " << used << ", Limit: " << limit 
              << ", Requesting: " << filesize << std::endl;

    if (used + filesize > limit) {
        std::cout << "[QuotaCheck] FAILED: Quota exceeded" << std::endl;
        return std::string(CODE_FAIL) + " Quota exceeded\n";
    }
    
    std::cout << "[QuotaCheck] SUCCESS: Quota OK" << std::endl;
    return std::string(CODE_OK) + " Quota OK\n";
}

bool FileIOHandler::checkDownloadPermission(const ClientSession& session, const std::string& filename) {
    // Kiểm tra xem file có thuộc về user hoặc được share cho user không
    // Gọi DBManager::checkPermission(...)
    return true; // Giả lập luôn có quyền
}