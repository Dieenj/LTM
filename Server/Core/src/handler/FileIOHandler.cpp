#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include "../../../../Common/Protocol.h"
#include <iostream>
#include <filesystem>

// Sử dụng namespace filesystem để tạo thư mục
namespace fs = std::filesystem;
const std::string STORAGE_DIR = "storage"; // Thư mục gốc lưu file

std::string FileIOHandler::handleQuotaCheck(const ClientSession& session, long filesize) {
    std::cout << "[QuotaCheck] User: " << session.username 
              << ", Auth: " << session.isAuthenticated 
              << ", FileSize: " << filesize << std::endl;
    
    // 1. Kiểm tra đã đăng nhập chưa
    if (!session.isAuthenticated) {
        std::cout << "[QuotaCheck] FAILED: Not logged in" << std::endl;
        return std::string(CODE_LOGIN_FAIL) + " Not logged in\n";
    }
    
    // 2. Tạo thư mục user nếu chưa có (Tránh lỗi hệ thống)
    std::string userPath = STORAGE_DIR + "/" + session.username;
    if (!fs::exists(userPath)) {
        try {
            fs::create_directories(userPath);
        } catch (...) {
            return std::string(CODE_FAIL) + " Server Storage Error\n";
        }
    }

    // 3. Lấy dung lượng đã dùng từ Database
    long used = DBManager::getInstance().getStorageUsed(session.username);
    long limit = 1073741824; // 1GB (Sau này có thể lấy từ DB)
    
    std::cout << "[QuotaCheck] Used: " << used << ", Limit: " << limit 
              << ", Requesting: " << filesize << std::endl;

    // 4. Kiểm tra xem có vượt quá giới hạn không
    if (used + filesize > limit) {
        std::cout << "[QuotaCheck] FAILED: Quota exceeded" << std::endl;
        return std::string(CODE_FAIL) + " Quota exceeded\n";
    }
    
    // 5. Trả về OK để Client bắt đầu upload
    std::cout << "[QuotaCheck] SUCCESS: Quota OK" << std::endl;
    return std::string(CODE_OK) + " Quota OK\n";
}

bool FileIOHandler::checkDownloadPermission(const ClientSession& session, const std::string& filename) {
    // Kiểm tra file vật lý có tồn tại không
    std::string filePath = STORAGE_DIR + "/" + session.username + "/" + filename;
    return fs::exists(filePath);
}