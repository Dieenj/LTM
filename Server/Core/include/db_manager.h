#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <vector>
#include <mysql/mysql.h>

// ===== EXISTING STRUCT - GIỮ NGUYÊN =====
struct FileRecord {
    std::string name;
    long size;
    std::string owner;
};

// ===== NEW STRUCT FOR FOLDER SHARE =====
struct FileRecordEx {
    long long file_id;
    long long owner_id;
    long long parent_id;
    std::string name;
    bool is_folder;
    long long size;
    std::string owner;
};

class DBManager {
public:
    static DBManager& getInstance() {
        static DBManager instance;
        return instance;
    }

    // ===== EXISTING FUNCTIONS - GIỮ NGUYÊN =====
    bool connect();
    void disconnect();
    bool checkUser(std::string user, std::string pass);
    std::vector<FileRecord> getFiles(std::string username);
    std::vector<FileRecord> getSharedFiles(std::string username);
    bool addFile(std::string filename, long filesize, std::string owner);
    long getStorageUsed(std::string username);
    bool shareFile(std::string filename, std::string ownerUsername, std::string targetUsername);
    bool deleteFile(std::string filename, std::string username);

    // ===== NEW FUNCTIONS FOR FOLDER SHARE =====
    
    // Lấy danh sách items trong một folder
    std::vector<FileRecordEx> getItemsInFolder(long long parent_id, std::string username);
    
    // Lấy toàn bộ cấu trúc folder (đệ quy)
    std::vector<FileRecordEx> getFolderStructure(long long folder_id, std::string username);
    
    // Tạo folder mới
    long long createFolder(std::string foldername, long long parent_id, std::string owner);
    
    // Tạo file trong folder
    long long createFileInFolder(std::string filename, long long parent_id, long long filesize, std::string owner);
    
    // Lấy thông tin file/folder
    FileRecordEx getFileInfo(long long file_id);
    
    // Share folder với user khác
    bool shareFolderWithUser(long long folder_id, std::string targetUsername);

private:
    DBManager() : conn(nullptr) {} 
    ~DBManager() { disconnect(); }
    MYSQL* conn;
    
    // ===== HELPER FUNCTION FOR RECURSIVE COLLECTION =====
    void collectFilesRecursive(long long parent_id, std::vector<FileRecordEx>& results, std::string username);
};

#endif