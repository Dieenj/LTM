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

std::string FolderShareHandler::generateSessionId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "FS_";
    for (int i = 0; i < 32; i++) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

std::string FolderShareHandler::buildRelativePath(long long file_id, 
                                                  const std::vector<FileRecordEx>& all_files) {
    std::vector<std::string> path_parts;
    long long current_id = file_id;
    
    std::map<long long, FileRecordEx> file_map;
    for (const auto& f : all_files) {
        file_map[f.file_id] = f;
    }
    
    while (current_id != -1 && file_map.find(current_id) != file_map.end()) {
        const auto& file = file_map[current_id];
        path_parts.insert(path_parts.begin(), file.name);
        current_id = file.parent_id;
        
        if (current_id == 1) break;
    }
    
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
    std::vector<FileRecordEx> folders;
    for (const auto& item : structure) {
        if (item.is_folder && item.file_id != session.source_folder_id) {
            folders.push_back(item);
        }
    }
    
    std::sort(folders.begin(), folders.end(), [](const FileRecordEx& a, const FileRecordEx& b) {
        return a.parent_id < b.parent_id;
    });
    
    for (const auto& folder : folders) {
        long long new_parent_id = session.old_to_new_id_map[folder.parent_id];
        
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

std::string FolderShareHandler::initiateFolderShare(const std::string& owner_username,
                                                    long long folder_id,
                                                    const std::string& recipient_username) {
    std::cout << "[FolderShare] Initiating share: folder_id=" << folder_id 
              << ", owner=" << owner_username 
              << ", recipient=" << recipient_username << std::endl;
    
    FileRecordEx folder_info = DBManager::getInstance().getFileInfo(folder_id);
    if (folder_info.file_id == -1 || !folder_info.is_folder) {
        std::cerr << "[FolderShare] Invalid folder_id: " << folder_id << std::endl;
        return "";
    }
    
    if (folder_info.owner != owner_username) {
        std::cerr << "[FolderShare] User " << owner_username << " does not own folder " << folder_id << std::endl;
        return "";
    }
    
    std::vector<FileRecordEx> structure = DBManager::getInstance().getFolderStructure(folder_id, owner_username);
    
    if (structure.empty()) {
        std::cerr << "[FolderShare] Empty folder or failed to get structure" << std::endl;
        return "";
    }
    
    FolderShareSession session;
    session.session_id = generateSessionId();
    session.owner_username = owner_username;
    session.source_folder_id = folder_id;
    session.recipient_username = recipient_username;
    session.total_files = 0;
    session.completed_files = 0;
    session.status = "pending";
    
    session.new_root_folder_id = DBManager::getInstance().createFolder(
        folder_info.name,
        -1,
        recipient_username
    );
    
    if (session.new_root_folder_id == -1) {
        std::cerr << "[FolderShare] Failed to create root folder" << std::endl;
        return "";
    }
    
    session.old_to_new_id_map[folder_id] = session.new_root_folder_id;
    
    createFolderStructure(session, structure);
    
    for (const auto& item : structure) {
        if (!item.is_folder) {
            FileTransferInfo info;
            info.old_file_id = item.file_id;
            info.new_file_id = -1;
            info.name = item.name;
            info.relative_path = buildRelativePath(item.file_id, structure);
            info.size_bytes = item.size;
            info.uploaded = false;
            
            session.files_to_transfer.push_back(info);
            session.total_files++;
        }
    }
    
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
    
    FileRecordEx old_file = DBManager::getInstance().getFileInfo(old_file_id);
    if (old_file.file_id == -1) {
        std::cerr << "[FolderShare] Old file not found: " << old_file_id << std::endl;
        return false;
    }
    
    long long new_parent_id = session->old_to_new_id_map[old_file.parent_id];
    
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
    
    std::string user_dir = STORAGE_DIR + "/" + session->recipient_username;
    
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
    
    for (const auto& pair : session->old_to_new_id_map) {
        long long new_file_id = pair.second;
        
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
