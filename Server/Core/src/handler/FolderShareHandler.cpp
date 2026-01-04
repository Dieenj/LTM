#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
const std::string STORAGE_DIR = "storage";

// ===== HELPER FUNCTIONS =====

std::string FolderShareHandler::generateSessionId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "FS_"; // Folder Share prefix
    for (int i = 0; i < 32; i++) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

std::string FolderShareHandler::buildRelativePath(long long file_id, 
                                                  const std::vector<FileRecordEx>& all_files) {
    std::vector<std::string> path_parts;
    long long current_id = file_id;
    
    // Map for quick lookup
    std::map<long long, FileRecordEx> file_map;
    for (const auto& f : all_files) {
        file_map[f.file_id] = f;
    }
    
    // Build path from file to root
    while (current_id != -1 && file_map.find(current_id) != file_map.end()) {
        const auto& file = file_map[current_id];
        path_parts.insert(path_parts.begin(), file.name);
        current_id = file.parent_id;
        
        // Stop at root folder (parent_id = 1)
        if (current_id == 1) break;
    }
    
    // Join with "/"
    std::string result;
    for (size_t i = 0; i < path_parts.size(); i++) {
        result += path_parts[i];
        if (i < path_parts.size() - 1) {
            result += "/";
        }
    }
    
    return result;
}

void FolderShareHandler::createFolderStructure(FolderShareSession& session,
                                              const std::vector<FileRecordEx>& structure) {
    // Sort folders by parent_id (BFS order)
    std::vector<FileRecordEx> folders;
    for (const auto& item : structure) {
        if (item.is_folder && item.file_id != session.source_folder_id) {
            folders.push_back(item);
        }
    }
    
    std::sort(folders.begin(), folders.end(), [](const FileRecordEx& a, const FileRecordEx& b) {
        return a.parent_id < b.parent_id;
    });
    
    // Create folders
    for (const auto& folder : folders) {
        // Find new parent ID
        long long new_parent_id = session.old_to_new_id_map[folder.parent_id];
        
        // Create folder in database
        long long new_folder_id = DBManager::getInstance().createFolder(
            folder.name,
            new_parent_id,
            session.recipient_username
        );
        
        if (new_folder_id != -1) {
            session.old_to_new_id_map[folder.file_id] = new_folder_id;
            std::cout << "[FolderShare] Created folder: " << folder.name 
                      << " (old_id=" << folder.file_id << ", new_id=" << new_folder_id << ")" << std::endl;
        }
    }
}

// ===== PUBLIC METHODS =====

std::string FolderShareHandler::initiateFolderShare(const std::string& owner_username,
                                                    long long folder_id,
                                                    const std::string& recipient_username) {
    std::cout << "[FolderShare] Initiating share: folder_id=" << folder_id 
              << ", owner=" << owner_username 
              << ", recipient=" << recipient_username << std::endl;
    
    // 1. Verify folder exists and belongs to owner
    FileRecordEx folder_info = DBManager::getInstance().getFileInfo(folder_id);
    if (folder_info.file_id == -1 || !folder_info.is_folder) {
        std::cerr << "[FolderShare] Invalid folder_id: " << folder_id << std::endl;
        return "";
    }
    
    if (folder_info.owner != owner_username) {
        std::cerr << "[FolderShare] User " << owner_username << " does not own folder " << folder_id << std::endl;
        return "";
    }
    
    // 2. Get folder structure
    std::vector<FileRecordEx> structure = DBManager::getInstance().getFolderStructure(folder_id, owner_username);
    
    if (structure.empty()) {
        std::cerr << "[FolderShare] Empty folder or failed to get structure" << std::endl;
        return "";
    }
    
    // 3. Create session
    FolderShareSession session;
    session.session_id = generateSessionId();
    session.owner_username = owner_username;
    session.source_folder_id = folder_id;
    session.recipient_username = recipient_username;
    session.total_files = 0;
    session.completed_files = 0;
    session.status = "pending";
    
    // 4. Create root folder for recipient
    session.new_root_folder_id = DBManager::getInstance().createFolder(
        folder_info.name,
        -1, // Root level (sẽ tạo với parent_id=1 trong DB)
        recipient_username
    );
    
    if (session.new_root_folder_id == -1) {
        std::cerr << "[FolderShare] Failed to create root folder" << std::endl;
        return "";
    }
    
    // Map old root -> new root
    session.old_to_new_id_map[folder_id] = session.new_root_folder_id;
    
    // 5. Create folder structure (only folders, no files yet)
    createFolderStructure(session, structure);
    
    // 6. Prepare file transfer list
    for (const auto& item : structure) {
        if (!item.is_folder) { // Only files need to be uploaded
            FileTransferInfo info;
            info.old_file_id = item.file_id;
            info.new_file_id = -1; // Will be created after upload
            info.name = item.name;
            info.relative_path = buildRelativePath(item.file_id, structure);
            info.size_bytes = item.size;
            info.uploaded = false;
            
            session.files_to_transfer.push_back(info);
            session.total_files++;
        }
    }
    
    // 7. Save session
    active_sessions[session.session_id] = session;
    
    std::cout << "[FolderShare] Session created: " << session.session_id 
              << ", total files: " << session.total_files << std::endl;
    
    return session.session_id;
}

FolderShareSession* FolderShareHandler::getSession(const std::string& session_id) {
    auto it = active_sessions.find(session_id);
    if (it != active_sessions.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool FolderShareHandler::receiveFile(const std::string& session_id,
                                    long long old_file_id,
                                    const char* file_data,
                                    size_t file_size) {
    auto session = getSession(session_id);
    if (!session) {
        std::cerr << "[FolderShare] Invalid session_id: " << session_id << std::endl;
        return false;
    }
    
    // Find file in transfer list
    FileTransferInfo* file_info = nullptr;
    for (auto& f : session->files_to_transfer) {
        if (f.old_file_id == old_file_id) {
            file_info = &f;
            break;
        }
    }
    
    if (!file_info) {
        std::cerr << "[FolderShare] File not in transfer list: " << old_file_id << std::endl;
        return false;
    }
    
    if (file_info->uploaded) {
        std::cerr << "[FolderShare] File already uploaded: " << old_file_id << std::endl;
        return false;
    }
    
    // Get old file info
    FileRecordEx old_file = DBManager::getInstance().getFileInfo(old_file_id);
    if (old_file.file_id == -1) {
        std::cerr << "[FolderShare] Old file not found: " << old_file_id << std::endl;
        return false;
    }
    
    // Find new parent ID
    long long new_parent_id = session->old_to_new_id_map[old_file.parent_id];
    
    // Create file record in database
    long long new_file_id = DBManager::getInstance().createFileInFolder(
        old_file.name,
        new_parent_id,
        file_size,
        session->recipient_username
    );
    
    if (new_file_id == -1) {
        std::cerr << "[FolderShare] Failed to create file record" << std::endl;
        return false;
    }
    
    // Save file to disk: storage/recipient_username/new_file_id
    std::string user_dir = STORAGE_DIR + "/" + session->recipient_username;
    
    // Create user directory if not exists
    try {
        fs::create_directories(user_dir);
    } catch (const std::exception& e) {
        std::cerr << "[FolderShare] Failed to create directory: " << e.what() << std::endl;
        return false;
    }
    
    std::string file_path = user_dir + "/" + std::to_string(new_file_id);
    std::ofstream out(file_path, std::ios::binary);
    if (!out) {
        std::cerr << "[FolderShare] Failed to create file: " << file_path << std::endl;
        return false;
    }
    
    out.write(file_data, file_size);
    out.close();
    
    // Update session
    file_info->new_file_id = new_file_id;
    file_info->uploaded = true;
    session->completed_files++;
    session->old_to_new_id_map[old_file_id] = new_file_id;
    session->status = "uploading";
    
    std::cout << "[FolderShare] File uploaded: " << old_file.name 
              << " (" << session->completed_files << "/" << session->total_files << ")" << std::endl;
    
    return true;
}

bool FolderShareHandler::isComplete(const std::string& session_id) {
    auto session = getSession(session_id);
    if (!session) return false;
    
    return session->completed_files >= session->total_files;
}

bool FolderShareHandler::finalize(const std::string& session_id) {
    auto session = getSession(session_id);
    if (!session) return false;
    
    // Share all folders and files with recipient
    for (const auto& pair : session->old_to_new_id_map) {
        long long new_file_id = pair.second;
        
        // Grant VIEW permission (permission_id = 1)
        DBManager::getInstance().shareFolderWithUser(new_file_id, session->recipient_username);
    }
    
    session->status = "completed";
    std::cout << "[FolderShare] Share completed: " << session_id << std::endl;
    
    return true;
}

void FolderShareHandler::cleanup(const std::string& session_id) {
    active_sessions.erase(session_id);
    std::cout << "[FolderShare] Session cleaned up: " << session_id << std::endl;
}

std::string FolderShareHandler::getProgress(const std::string& session_id) {
    auto session = getSession(session_id);
    if (!session) {
        return "ERROR: Session not found";
    }
    
    std::stringstream ss;
    ss << "STATUS:" << session->status << "|";
    ss << "COMPLETED:" << session->completed_files << "|";
    ss << "TOTAL:" << session->total_files << "|";
    
    int percentage = (session->total_files > 0) 
        ? (session->completed_files * 100 / session->total_files) 
        : 0;
    ss << "PROGRESS:" << percentage << "%";
    
    return ss.str();
}

// ===== CMD HANDLER EXTENSIONS =====

std::string CmdHandler::handleShareFolder(const ClientSession& session, 
                                         long long folder_id, 
                                         const std::string& targetUser) {
    if (!session.isAuthenticated) {
        return "401 Please login first\n";
    }
    
    // Initiate folder share
    std::string session_id = FolderShareHandler::getInstance().initiateFolderShare(
        session.username,
        folder_id,
        targetUser
    );
    
    if (session_id.empty()) {
        return "500 Failed to initiate folder share\n";
    }
    
    // Get session info
    auto share_session = FolderShareHandler::getInstance().getSession(session_id);
    if (!share_session) {
        return "500 Session creation failed\n";
    }
    
    // Build response with file list
    std::stringstream response;
    response << "200 Folder share initiated\n";
    response << "SESSION_ID:" << session_id << "\n";
    response << "TOTAL_FILES:" << share_session->total_files << "\n";
    response << "FILES:\n";
    
    for (const auto& file : share_session->files_to_transfer) {
        response << file.old_file_id << "|" 
                << file.name << "|" 
                << file.relative_path << "|" 
                << file.size_bytes << "\n";
    }
    
    return response.str();
}

std::string CmdHandler::handleGetFolderStructure(const ClientSession& session, 
                                                 long long folder_id) {
    if (!session.isAuthenticated) {
        return "401 Please login first\n";
    }
    
    // Get folder structure
    auto structure = DBManager::getInstance().getFolderStructure(folder_id, session.username);
    
    if (structure.empty()) {
        return "404 Folder not found or empty\n";
    }
    
    // Build response
    std::stringstream response;
    response << "200 OK\n";
    response << "FOLDER_ID:" << folder_id << "\n";
    response << "ITEMS:\n";
    
    for (const auto& item : structure) {
        response << item.file_id << "|"
                << item.name << "|"
                << (item.is_folder ? "FOLDER" : "FILE") << "|"
                << item.size << "|"
                << item.parent_id << "\n";
    }
    
    return response.str();
}