#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include "../../../../Common/Protocol.h"
#include <iostream>
#include <sstream>
#include <vector>

std::string CmdHandler::handleList(const ClientSession& session, long long parent_id) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }

    auto files = DBManager::getInstance().getFiles(session.username, parent_id);
    
    if (files.empty()) {
        return "210 Empty folder\n";
    }

    std::string response = "";
    for (const auto& f : files) {
        std::string type = f.is_folder ? "Folder" : "File";
        response += f.name + "|" + type + "|" + std::to_string(f.size) + "|" + f.owner + "|" + std::to_string(f.file_id) + "\n";
    }
    return response;
}

std::string CmdHandler::handleListShared(const ClientSession& session, long long parent_id) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }

    std::vector<FileRecord> files;
    
    if (parent_id < 0) {
        files = DBManager::getInstance().getSharedFiles(session.username);
    } else {
        files = DBManager::getInstance().getSharedFiles(session.username, parent_id);
    }
    
    if (files.empty()) {
        return "210 No shared files\n";
    }

    std::string response = "";
    for (const auto& f : files) {
        std::string type = f.is_folder ? "Folder" : "File";
        response += f.name + "|" + type + "|" + std::to_string(f.size) + "|" + f.owner + "|" + std::to_string(f.file_id) + "\n";
    }
    return response;
}

std::string CmdHandler::handleSearch(const ClientSession& session, const std::string& keyword) {
    if (!session.isAuthenticated) return std::string(CODE_FAIL) + " Please login first\n";

    return "Result_File_1.txt|1024|admin\n";
}

std::string CmdHandler::handleShare(const ClientSession& session, const std::string& filename, const std::string& targetUser) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }

    bool success = DBManager::getInstance().shareFile(filename, session.username, targetUser);
    
    if (success) {
        return std::string(CODE_OK) + " File '" + filename + "' shared with " + targetUser + "\n";
    } else {
        return std::string(CODE_FAIL) + " Failed to share file. Check if file exists and target user is valid\n";
    }
}

std::string CmdHandler::handleDelete(const ClientSession& session, const std::string& filename) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }

    bool success = DBManager::getInstance().deleteFile(filename, session.username);
    
    if (success) {
        return std::string(CODE_OK) + " File '" + filename + "' deleted successfully\n";
    } else {
        return std::string(CODE_FAIL) + " Failed to delete file. You must be the owner to delete this file\n";
    }
}

std::string CmdHandler::handleShareFolder(const ClientSession& session, 
                                          long long folder_id, 
                                          const std::string& targetUser) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }
    
    std::cout << "[CmdHandler] Sharing folder " << folder_id 
              << " from " << session.username 
              << " to " << targetUser << std::endl;
    
    bool success = DBManager::getInstance().shareFolderWithUser(folder_id, targetUser);
    
    if (success) {
        std::cout << "[CmdHandler] Folder share successful" << std::endl;
        return std::string(CODE_OK) + " Folder shared successfully\n";
    } else {
        std::cerr << "[CmdHandler] Folder share failed" << std::endl;
        return std::string(CODE_FAIL) + " Failed to share folder\n";
    }
}

std::string CmdHandler::handleGetFolderStructure(const ClientSession& session, 
                                                 long long folder_id) {
    if (!session.isAuthenticated) {
        return std::string(CODE_FAIL) + " Please login first\n";
    }
    
    auto structure = DBManager::getInstance().getFolderStructure(folder_id, session.username);
    
    if (structure.empty()) {
        return "404 Folder not found or empty\n";
    }
    
    std::stringstream response;
    response << CODE_OK << " OK\n";
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
