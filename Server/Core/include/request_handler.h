#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <vector>
#include <map>
#include "server.h" // Chứa struct ClientSession

// ===== EXISTING HANDLERS - GIỮ NGUYÊN =====

// Xử lý xác thực (Login, Register)
class AuthHandler {
public:
    static std::string handleUser(int fd, ClientSession& session, const std::string& username);
    static std::string handlePass(int fd, ClientSession& session, const std::string& password);
    static std::string handleRegister(const std::string& username, const std::string& password);
};

// Xử lý lệnh quản lý file (List, Search, Share)
class CmdHandler {
public:
    static std::string handleList(const ClientSession& session);
    static std::string handleListShared(const ClientSession& session);
    static std::string handleSearch(const ClientSession& session, const std::string& keyword);
    static std::string handleShare(const ClientSession& session, const std::string& filename, const std::string& targetUser);
    static std::string handleDelete(const ClientSession& session, const std::string& filename);
    
    // ===== NEW: FOLDER SHARE COMMANDS =====
    static std::string handleShareFolder(const ClientSession& session, long long folder_id, const std::string& targetUser);
    static std::string handleGetFolderStructure(const ClientSession& session, long long folder_id);
};

// Xử lý chuẩn bị I/O (Quota check, Permission check trước khi upload/download)
class FileIOHandler {
public:
    static std::string handleQuotaCheck(const ClientSession& session, long filesize);
    static bool checkDownloadPermission(const ClientSession& session, const std::string& filename);
};

// ===== NEW: FOLDER SHARE HANDLER =====

struct FileTransferInfo {
    long long old_file_id;
    long long new_file_id;
    std::string name;
    std::string relative_path;
    long long size_bytes;
    bool uploaded;
};

struct FolderShareSession {
    std::string session_id;
    std::string owner_username;
    long long source_folder_id;
    std::string recipient_username;
    long long new_root_folder_id;
    std::vector<FileTransferInfo> files_to_transfer;
    std::map<long long, long long> old_to_new_id_map;
    int total_files;
    int completed_files;
    std::string status; // "pending", "uploading", "completed", "failed"
};

class FolderShareHandler {
public:
    // Singleton
    static FolderShareHandler& getInstance() {
        static FolderShareHandler instance;
        return instance;
    }
    
    // Khởi tạo folder share và trả về session_id
    std::string initiateFolderShare(const std::string& owner_username, 
                                    long long folder_id, 
                                    const std::string& recipient_username);
    
    // Lấy session info
    FolderShareSession* getSession(const std::string& session_id);
    
    // Nhận file từ client
    bool receiveFile(const std::string& session_id, 
                    long long old_file_id,
                    const char* file_data,
                    size_t file_size);
    
    // Kiểm tra hoàn thành
    bool isComplete(const std::string& session_id);
    
    // Finalize share (tạo permissions)
    bool finalize(const std::string& session_id);
    
    // Cleanup session
    void cleanup(const std::string& session_id);
    
    // Get progress
    std::string getProgress(const std::string& session_id);

private:
    FolderShareHandler() {}
    ~FolderShareHandler() {}
    
    std::map<std::string, FolderShareSession> active_sessions;
    
    void createFolderStructure(FolderShareSession& session, 
                              const std::vector<FileRecordEx>& structure);
    
    std::string buildRelativePath(long long file_id, 
                                  const std::vector<FileRecordEx>& all_files);
    
    std::string generateSessionId();
};

#endif // REQUEST_HANDLER_H