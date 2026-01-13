#include "../../include/db_manager.h"
#include "../../include/db_config.h"
#include <iostream>
#include <sstream>
#include <openssl/sha.h>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <cerrno>
#include <chrono>
#include <thread>

// Define STORAGE_PATH if not already defined
#ifndef STORAGE_PATH
#define STORAGE_PATH "./storage/"
#endif

std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)str.c_str(), str.size(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool DBManager::connect() {
    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "[DB] mysql_init() failed" << std::endl;
        return false;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, 
                            DB_NAME, DB_PORT, nullptr, 0)) {
        std::cerr << "[DB] Connection failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    return true;
}

void DBManager::disconnect() {
    if (conn) {
        mysql_close(conn);
        conn = nullptr;
    }
}

bool DBManager::checkUser(std::string user, std::string pass) {
    if (!conn) return false;

    std::string hashed_pass = sha256(pass);
    
    std::string query = "SELECT user_id FROM USERS WHERE username = '" + user + 
                       "' AND password_hash = '" + hashed_pass + "'";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;

    bool found = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return found;
}

bool DBManager::registerUser(std::string username, std::string password) {
    if (!conn) {
        std::cerr << "[DB] Not connected to database" << std::endl;
        return false;
    }

    std::string check_query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, check_query.c_str())) {
        std::cerr << "[DB] Check query failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "[DB] Failed to get result: " << mysql_error(conn) << std::endl;
        return false;
    }

    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);

    if (exists) {
        std::cerr << "[DB] Username '" << username << "' already exists" << std::endl;
        return false;
    }

    std::string hashed_pass = sha256(password);
    
    std::string insert_query = "INSERT INTO USERS (username, password_hash, created_at) VALUES ('" + 
                               username + "', '" + hashed_pass + "', NOW())";
    
    if (mysql_query(conn, insert_query.c_str())) {
        std::cerr << "[DB] Insert failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    return true;
}

std::vector<FileRecord> DBManager::getFiles(std::string username, long long parent_id) {
    std::vector<FileRecord> list;
    if (!conn) return list;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return list;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return list;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return list;
    }
    std::string user_id = row[0];
    mysql_free_result(result);

    if (parent_id == 0) {
        query = "SELECT f.name, f.size_bytes, u.username, f.created_at, f.file_id, f.is_folder "
                "FROM FILES f "
                "JOIN USERS u ON f.owner_id = u.user_id "
                "WHERE f.file_id != 1 "
                "AND f.owner_id = " + user_id + " "
                "AND f.parent_id IS NULL "
                "AND f.is_deleted = FALSE "
                "ORDER BY f.is_folder DESC, f.created_at DESC";
    } else {
        query = "SELECT f.name, f.size_bytes, u.username, f.created_at, f.file_id, f.is_folder "
                "FROM FILES f "
                "JOIN USERS u ON f.owner_id = u.user_id "
                "WHERE f.file_id != 1 "
                "AND f.owner_id = " + user_id + " "
                "AND f.parent_id = " + std::to_string(parent_id) + " "
                "AND f.is_deleted = FALSE "
                "ORDER BY f.is_folder DESC, f.created_at DESC";
    }

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return list;
    }

    result = mysql_store_result(conn);
    if (!result) return list;

    while ((row = mysql_fetch_row(result))) {
        FileRecord rec;
        rec.name = row[0] ? row[0] : "";
        rec.size = row[1] ? std::stol(row[1]) : 0;
        rec.owner = row[2] ? row[2] : "";
        rec.file_id = row[4] ? std::stoll(row[4]) : 0;
        rec.is_folder = row[5] && std::string(row[5]) == "1";
        
        if (rec.is_folder) {
            std::string size_query = 
                "SELECT COALESCE(SUM(size_bytes), 0) FROM FILES "
                "WHERE parent_id = " + std::to_string(rec.file_id) + " "
                "AND is_folder = FALSE AND is_deleted = FALSE";
            
            if (mysql_query(conn, size_query.c_str()) == 0) {
                MYSQL_RES* size_result = mysql_store_result(conn);
                if (size_result) {
                    MYSQL_ROW size_row = mysql_fetch_row(size_result);
                    if (size_row && size_row[0]) {
                        rec.size = std::stol(size_row[0]);
                    }
                    mysql_free_result(size_result);
                }
            }
        }
        
        list.push_back(rec);
    }

    mysql_free_result(result);
    return list;
}

std::vector<FileRecord> DBManager::getSharedFiles(std::string username) {
    std::vector<FileRecord> list;
    if (!conn) return list;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return list;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return list;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return list;
    }
    std::string user_id = row[0];
    mysql_free_result(result);


    query = "SELECT f.name, f.size_bytes, u.username, f.file_id, f.is_folder "
            "FROM FILES f "
            "JOIN USERS u ON f.owner_id = u.user_id "
            "JOIN SHAREDFILES sf ON f.file_id = sf.file_id "
            "WHERE sf.user_id = " + user_id + " "
            "AND f.owner_id != " + user_id + " "
            "AND f.is_deleted = FALSE "
            "ORDER BY f.is_folder DESC, sf.shared_at DESC";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return list;
    }

    result = mysql_store_result(conn);
    if (!result) return list;

    while ((row = mysql_fetch_row(result))) {
        FileRecord rec;
        rec.name = row[0] ? row[0] : "";
        rec.size = row[1] ? std::stol(row[1]) : 0;
        rec.owner = row[2] ? row[2] : "";
        rec.file_id = row[3] ? std::stoll(row[3]) : 0;
        rec.is_folder = row[4] && std::string(row[4]) == "1";
        
        if (rec.is_folder) {
            std::string size_query = 
                "SELECT COALESCE(SUM(size_bytes), 0) FROM FILES "
                "WHERE parent_id = " + std::to_string(rec.file_id) + " "
                "AND is_folder = FALSE AND is_deleted = FALSE";
            
            if (mysql_query(conn, size_query.c_str()) == 0) {
                MYSQL_RES* size_result = mysql_store_result(conn);
                if (size_result) {
                    MYSQL_ROW size_row = mysql_fetch_row(size_result);
                    if (size_row && size_row[0]) {
                        rec.size = std::stol(size_row[0]);
                    }
                    mysql_free_result(size_result);
                }
            }
        }
        
        list.push_back(rec);
    }

    mysql_free_result(result);
    return list;
}

std::vector<FileRecord> DBManager::getSharedFiles(std::string username, long long parent_id) {
    std::vector<FileRecord> list;
    if (!conn) return list;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return list;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return list;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return list;
    }
    std::string user_id = row[0];
    mysql_free_result(result);

    if (parent_id > 0) {
        if (!hasSharedAccess(parent_id, username)) {
            std::cerr << "[DB] User '" << username << "' has no access to folder " << parent_id << std::endl;
            return list;
        }
    }

    query = "SELECT f.name, f.size_bytes, u.username, f.file_id, f.is_folder "
            "FROM FILES f "
            "JOIN USERS u ON f.owner_id = u.user_id "
            "WHERE f.parent_id = " + std::to_string(parent_id) + " "
            "AND f.is_deleted = FALSE "
            "ORDER BY f.is_folder DESC, f.created_at DESC";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return list;
    }

    result = mysql_store_result(conn);
    if (!result) return list;

    while ((row = mysql_fetch_row(result))) {
        FileRecord rec;
        rec.name = row[0] ? row[0] : "";
        rec.size = row[1] ? std::stol(row[1]) : 0;
        rec.owner = row[2] ? row[2] : "";
        rec.file_id = row[3] ? std::stoll(row[3]) : 0;
        rec.is_folder = row[4] && std::string(row[4]) == "1";
        
        if (rec.is_folder) {
            std::string size_query = 
                "SELECT COALESCE(SUM(size_bytes), 0) FROM FILES "
                "WHERE parent_id = " + std::to_string(rec.file_id) + " "
                "AND is_folder = FALSE AND is_deleted = FALSE";
            
            if (mysql_query(conn, size_query.c_str()) == 0) {
                MYSQL_RES* size_result = mysql_store_result(conn);
                if (size_result) {
                    MYSQL_ROW size_row = mysql_fetch_row(size_result);
                    if (size_row && size_row[0]) {
                        rec.size = std::stol(size_row[0]);
                    }
                    mysql_free_result(size_result);
                }
            }
        }
        
        list.push_back(rec);
    }

    mysql_free_result(result);
    std::cout << "[DB] Retrieved " << list.size() << " items in shared folder " << parent_id 
              << " for user '" << username << "'" << std::endl;
    return list;
}

bool DBManager::hasSharedAccess(long long file_id, std::string username) {
    if (!conn) return false;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return false;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    std::string user_id = row[0];
    mysql_free_result(result);

    long long current_id = file_id;
    
    while (current_id > 0) {
        query = "SELECT COUNT(*) FROM SHAREDFILES "
                "WHERE file_id = " + std::to_string(current_id) + " "
                "AND user_id = " + user_id;
        
        if (mysql_query(conn, query.c_str())) return false;
        
        result = mysql_store_result(conn);
        if (!result) return false;
        
        row = mysql_fetch_row(result);
        if (row && row[0] && std::stoi(row[0]) > 0) {
            mysql_free_result(result);
            return true;
        }
        mysql_free_result(result);
        
        query = "SELECT parent_id FROM FILES WHERE file_id = " + std::to_string(current_id);
        if (mysql_query(conn, query.c_str())) return false;
        
        result = mysql_store_result(conn);
        if (!result) return false;
        
        row = mysql_fetch_row(result);
        if (!row || !row[0]) {
            mysql_free_result(result);
            break;
        }
        
        current_id = std::stoll(row[0]);
        mysql_free_result(result);
    }
    
    query = "SELECT COUNT(*) FROM FILES f "
            "JOIN USERS u ON f.owner_id = u.user_id "
            "WHERE f.file_id = " + std::to_string(file_id) + " "
            "AND u.username = '" + username + "'";
    
    if (mysql_query(conn, query.c_str())) return false;
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    bool is_owner = (row && row[0] && std::stoi(row[0]) > 0);
    mysql_free_result(result);
    
    return is_owner;
}

bool DBManager::addFile(std::string filename, long filesize, std::string owner, long long parent_id) {
    if (!conn) return false;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + owner + "'";
    if (mysql_query(conn, query.c_str())) return false;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    std::string owner_id = row[0];
    mysql_free_result(result);

    std::stringstream ss;
    if (parent_id == 0) {
        ss << "INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES ("
           << owner_id << ", NULL, '" << filename << "', FALSE, " << filesize << ") "
           << "ON DUPLICATE KEY UPDATE size_bytes = " << filesize 
           << ", is_deleted = FALSE, updated_at = NOW()";
    } else {
        ss << "INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES ("
           << owner_id << ", " << parent_id << ", '" << filename << "', FALSE, " << filesize << ") "
           << "ON DUPLICATE KEY UPDATE size_bytes = " << filesize 
           << ", is_deleted = FALSE, updated_at = NOW()";
    }
    
    if (mysql_query(conn, ss.str().c_str())) {
        std::cerr << "[DB] Insert failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::cout << "[DB] File '" << filename << "' saved to database (parent_id: " << parent_id << ")" << std::endl;
    return true;
}

long DBManager::getStorageUsed(std::string username) {
    if (!conn) return 0;

    std::string query = "SELECT COALESCE(SUM(f.size_bytes), 0) FROM FILES f "
                       "JOIN USERS u ON f.owner_id = u.user_id "
                       "WHERE u.username = '" + username + "' AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) return 0;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return 0;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    long used = row && row[0] ? std::stol(row[0]) : 0;
    
    mysql_free_result(result);
    return used;
}

bool DBManager::shareFile(std::string filename, std::string ownerUsername, std::string targetUsername) {
    if (!conn) return false;

    std::cout << "[DB] shareFile called: file='" << filename << "' owner='" << ownerUsername << "' target='" << targetUsername << "'" << std::endl;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + ownerUsername + "'";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Get owner_id failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    std::string owner_id = row[0];
    mysql_free_result(result);

    query = "SELECT user_id FROM USERS WHERE username = '" + targetUsername + "'";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Get target_user_id failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        std::cerr << "[DB] Target user not found: " << targetUsername << std::endl;
        return false;
    }
    std::string target_user_id = row[0];
    mysql_free_result(result);

    query = "SELECT file_id FROM FILES WHERE name = '" + filename + "' AND owner_id = " + owner_id + " AND is_deleted = FALSE";
    std::cout << "[DB] Searching for file: query='" << query << "'" << std::endl;
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Get file_id failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        std::cerr << "[DB] File not found or not owned by user: file='" << filename << "' owner_id=" << owner_id << std::endl;
        return false;
    }
    std::string file_id = row[0];
    std::cout << "[DB] Found file_id: " << file_id << std::endl;
    mysql_free_result(result);

    query = "INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES (" 
            + file_id + ", " + target_user_id + ", 1) "
            "ON DUPLICATE KEY UPDATE shared_at = CURRENT_TIMESTAMP";
    
    std::cout << "[DB] Executing share insert: " << query << std::endl;
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Share file failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::cout << "[DB] File '" << filename << "' shared from " << ownerUsername 
              << " to " << targetUsername << std::endl;
    return true;
}

bool DBManager::deleteFile(std::string filename, std::string username) {
    if (!conn) return false;

    std::string query = "UPDATE FILES f "
                       "JOIN USERS u ON f.owner_id = u.user_id "
                       "SET f.is_deleted = TRUE "
                       "WHERE f.name = '" + filename + "' "
                       "AND u.username = '" + username + "' "
                       "AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Delete file failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    if (mysql_affected_rows(conn) == 0) {
        std::cerr << "[DB] File not found or user is not the owner" << std::endl;
        return false;
    }

    std::cout << "[DB] File '" << filename << "' deleted by " << username << std::endl;
    return true;
}

bool DBManager::renameFile(long long fileId, std::string newName, std::string username) {
    if (!conn) return false;

    // First, get the file info (old name, type, owner verification)
    std::string checkQuery = "SELECT f.name, f.is_folder FROM FILES f "
                            "JOIN USERS u ON f.owner_id = u.user_id "
                            "WHERE f.file_id = " + std::to_string(fileId) + " "
                            "AND u.username = '" + username + "' "
                            "AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, checkQuery.c_str())) {
        std::cerr << "[DB] Failed to check item: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result || mysql_num_rows(result) == 0) {
        if (result) mysql_free_result(result);
        std::cerr << "[DB] Item not found or user is not the owner" << std::endl;
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    std::string oldName = row[0] ? row[0] : "";
    bool isFolder = (row[1] && std::string(row[1]) == "1");
    mysql_free_result(result);
    
    std::string itemType = isFolder ? "FOLDER" : "FILE";
    std::cout << "[DB RENAME " << itemType << "] File ID: " << fileId << ", Renaming: '" << oldName << "' -> '" << newName << "' by user: " << username << std::endl;

    // Handle physical rename based on type
    if (isFolder) {
        // FOLDER: Check if physical directory exists
        std::string oldPath = std::string(STORAGE_PATH) + oldName;
        std::string newPath = std::string(STORAGE_PATH) + newName;
        
        std::cout << "[DB RENAME FOLDER] Physical path: " << oldPath << " -> " << newPath << std::endl;
        
        struct stat st;
        if (stat(oldPath.c_str(), &st) == 0) {
            // Physical folder exists - rename it
            if (!S_ISDIR(st.st_mode)) {
                std::cerr << "[DB RENAME FOLDER] ERROR: Expected folder but found file!" << std::endl;
                return false;
            }
            
            // Rename physical folder first
            if (std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
                std::cerr << "[DB RENAME FOLDER] Physical rename FAILED: " << strerror(errno) << std::endl;
                return false;
            }
            std::cout << "[DB RENAME FOLDER] Physical rename successful" << std::endl;
        } else {
            // Physical folder doesn't exist - just update database (empty folder from schema)
            std::cout << "[DB RENAME FOLDER] Physical folder not found, updating database only (empty folder)" << std::endl;
        }
    } else {
        // FILE: Only update database, no physical rename needed
        std::cout << "[DB RENAME FILE] Skipping physical rename for file (only updating database)" << std::endl;
    }

    // Update database
    std::string updateQuery = "UPDATE FILES f "
                             "JOIN USERS u ON f.owner_id = u.user_id "
                             "SET f.name = '" + newName + "' "
                             "WHERE f.file_id = " + std::to_string(fileId) + " "
                             "AND u.username = '" + username + "' "
                             "AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, updateQuery.c_str())) {
        std::cerr << "[DB RENAME " << itemType << "] Database update failed: " << mysql_error(conn) << std::endl;
        // Rollback physical rename if it was a folder
        if (isFolder) {
            std::string oldPath = std::string(STORAGE_PATH) + oldName;
            std::string newPath = std::string(STORAGE_PATH) + newName;
            std::rename(newPath.c_str(), oldPath.c_str());
            std::cerr << "[DB RENAME " << itemType << "] Rolled back physical rename" << std::endl;
        }
        return false;
    }

    if (mysql_affected_rows(conn) == 0) {
        std::cerr << "[DB RENAME " << itemType << "] No rows affected" << std::endl;
        // Rollback physical rename if it was a folder
        if (isFolder) {
            std::string oldPath = std::string(STORAGE_PATH) + oldName;
            std::string newPath = std::string(STORAGE_PATH) + newName;
            std::rename(newPath.c_str(), oldPath.c_str());
            std::cerr << "[DB RENAME " << itemType << "] Rolled back physical rename" << std::endl;
        }
        return false;
    }
    
    std::cout << "[DB RENAME " << itemType << "] Database updated successfully" << std::endl;

    std::cout << "[DB RENAME " << itemType << "] SUCCESS: '" << oldName << "' -> '" << newName << "'" << std::endl;
    return true;
}

std::vector<FileRecordEx> DBManager::getItemsInFolder(long long parent_id, std::string username) {
    std::vector<FileRecordEx> list;
    if (!conn) return list;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return list;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return list;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return list;
    }
    std::string user_id = row[0];
    mysql_free_result(result);

    std::stringstream ss;
    if (parent_id == -1) {
        ss << "SELECT file_id, owner_id, parent_id, name, is_folder, size_bytes, created_at "
           << "FROM FILES WHERE parent_id = 1 AND owner_id = " << user_id 
           << " AND is_deleted = FALSE ORDER BY is_folder DESC, name ASC";
    } else {
        ss << "SELECT file_id, owner_id, parent_id, name, is_folder, size_bytes, created_at "
           << "FROM FILES WHERE parent_id = " << parent_id 
           << " AND is_deleted = FALSE ORDER BY is_folder DESC, name ASC";
    }
    
    if (mysql_query(conn, ss.str().c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return list;
    }

    result = mysql_store_result(conn);
    if (!result) return list;

    while ((row = mysql_fetch_row(result))) {
        FileRecordEx rec;
        rec.file_id = row[0] ? std::stoll(row[0]) : 0;
        rec.owner_id = row[1] ? std::stoll(row[1]) : 0;
        rec.parent_id = row[2] ? std::stoll(row[2]) : -1;
        rec.name = row[3] ? row[3] : "";
        rec.is_folder = row[4] && std::string(row[4]) == "1";
        rec.size = row[5] ? std::stoll(row[5]) : 0;
        rec.owner = username;
        list.push_back(rec);
    }

    mysql_free_result(result);
    std::cout << "[DB] Retrieved " << list.size() << " items in folder (parent_id=" 
              << parent_id << ")" << std::endl;
    return list;
}

void DBManager::collectFilesRecursive(long long parent_id, std::vector<FileRecordEx>& results, std::string username) {
    auto items = getItemsInFolder(parent_id, username);
    
    for (const auto& item : items) {
        results.push_back(item);
        
        if (item.is_folder) {
            collectFilesRecursive(item.file_id, results, username);
        }
    }
}

std::vector<FileRecordEx> DBManager::getFolderStructure(long long folder_id, std::string username) {
    std::vector<FileRecordEx> allFiles;
    if (!conn) return allFiles;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) return allFiles;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return allFiles;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return allFiles;
    }
    std::string user_id = row[0];
    mysql_free_result(result);

    query = "SELECT file_id FROM FILES WHERE file_id = " + std::to_string(folder_id) +
            " AND owner_id = " + user_id + " AND is_folder = TRUE AND is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) return allFiles;
    result = mysql_store_result(conn);
    if (!result || mysql_num_rows(result) == 0) {
        if (result) mysql_free_result(result);
        std::cerr << "[DB] Folder not found or not owned by user" << std::endl;
        return allFiles;
    }
    mysql_free_result(result);

    collectFilesRecursive(folder_id, allFiles, username);
    
    std::cout << "[DB] Collected " << allFiles.size() << " items in folder structure" << std::endl;
    return allFiles;
}

long long DBManager::createFolder(std::string foldername, long long parent_id, std::string owner) {
    if (!conn) return -1;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + owner + "'";
    if (mysql_query(conn, query.c_str())) return -1;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return -1;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return -1;
    }
    std::string owner_id = row[0];
    mysql_free_result(result);

    std::stringstream ss;
    if (parent_id == -1 || parent_id == 0) {
        // Root level folder - parent is NULL or 1
        ss << "INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES ("
           << owner_id << ", NULL, '" << foldername << "', TRUE, 0)";
    } else {
        ss << "INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES ("
           << owner_id << ", " << parent_id << ", '" << foldername << "', TRUE, 0)";
    }
    
    if (mysql_query(conn, ss.str().c_str())) {
        std::cerr << "[DB] Create folder failed: " << mysql_error(conn) << std::endl;
        return -1;
    }

    long long new_folder_id = mysql_insert_id(conn);
    std::cout << "[DB] Folder '" << foldername << "' created with ID: " << new_folder_id << std::endl;
    return new_folder_id;
}

long long DBManager::createFileInFolder(std::string filename, long long parent_id, long long filesize, std::string owner) {
    if (!conn) return -1;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + owner + "'";
    if (mysql_query(conn, query.c_str())) return -1;
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return -1;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return -1;
    }
    std::string owner_id = row[0];
    mysql_free_result(result);

    std::stringstream ss;
    ss << "INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES ("
       << owner_id << ", " << parent_id << ", '" << filename << "', FALSE, " << filesize << ")";
    
    if (mysql_query(conn, ss.str().c_str())) {
        std::cerr << "[DB] Create file failed: " << mysql_error(conn) << std::endl;
        return -1;
    }

    long long new_file_id = mysql_insert_id(conn);
    std::cout << "[DB] File '" << filename << "' created in folder " << parent_id 
              << " with ID: " << new_file_id << std::endl;
    return new_file_id;
}

FileRecordEx DBManager::getFileInfo(long long file_id) {
    FileRecordEx rec;
    rec.file_id = -1;
    if (!conn) return rec;

    std::string query = "SELECT f.file_id, f.owner_id, f.parent_id, f.name, f.is_folder, "
                       "f.size_bytes, u.username FROM FILES f "
                       "JOIN USERS u ON f.owner_id = u.user_id "
                       "WHERE f.file_id = " + std::to_string(file_id);
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Query failed: " << mysql_error(conn) << std::endl;
        return rec;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return rec;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        rec.file_id = std::stoll(row[0]);
        rec.owner_id = std::stoll(row[1]);
        rec.parent_id = row[2] ? std::stoll(row[2]) : -1;
        rec.name = row[3] ? row[3] : "";
        rec.is_folder = row[4] && std::string(row[4]) == "1";
        rec.size = row[5] ? std::stoll(row[5]) : 0;
        rec.owner = row[6] ? row[6] : "";
    }
    
    mysql_free_result(result);
    return rec;
}

bool DBManager::shareFolderWithUser(long long folder_id, std::string targetUsername) {
    if (!conn) return false;

    std::cout << "[DB] shareFolderWithUser called: folder_id=" << folder_id << " target='" << targetUsername << "'" << std::endl;

    std::string query = "SELECT user_id FROM USERS WHERE username = '" + targetUsername + "'";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Get target_user_id failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        std::cerr << "[DB] Target user not found: " << targetUsername << std::endl;
        return false;
    }
    std::string target_user_id = row[0];
    mysql_free_result(result);
    std::cout << "[DB] Found target user_id: " << target_user_id << std::endl;

    query = "INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES (" 
            + std::to_string(folder_id) + ", " + target_user_id + ", 1) "
            "ON DUPLICATE KEY UPDATE shared_at = CURRENT_TIMESTAMP";
    
    std::cout << "[DB] Executing folder share: " << query << std::endl;
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] Share folder failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::cout << "[DB] Folder (ID=" << folder_id << ") shared with " << targetUsername 
              << " (recursive access via hasSharedAccess)" << std::endl;
    return true;
}

bool DBManager::isFileSharedWithUser(std::string filename, std::string username) {
    if (!conn) return false;
    
    std::string query = "SELECT file_id FROM FILES WHERE name = '" + filename + "' AND is_deleted = FALSE";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] isFileSharedWithUser query failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    long long file_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    query = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    std::string user_id = row[0];
    mysql_free_result(result);
    
    query = "SELECT COUNT(*) FROM SHAREDFILES WHERE file_id = " + std::to_string(file_id) 
            + " AND user_id = " + user_id;
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    bool isShared = (row && std::stoi(row[0]) > 0);
    mysql_free_result(result);
    
    return isShared;
}

std::string DBManager::getFileOwner(std::string filename) {
    if (!conn) return "";
    
    std::string query = "SELECT u.username FROM FILES f "
                       "JOIN USERS u ON f.owner_id = u.user_id "
                       "WHERE f.name = '" + filename + "' AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] getFileOwner query failed: " << mysql_error(conn) << std::endl;
        return "";
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return "";
    
    MYSQL_ROW row = mysql_fetch_row(result);
    std::string owner = "";
    if (row && row[0]) {
        owner = row[0];
    }
    mysql_free_result(result);
    
    return owner;
}

// ===== SHARE CODE FUNCTIONS =====

// Helper function to generate unique share code
static std::string generateUniqueCode(long long owner_id, long long file_id, int attempt) {
    // Combine: owner_id + file_id + timestamp + random + attempt
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    
    // Create data string to hash
    std::string data = std::to_string(owner_id) + "_" + 
                      std::to_string(file_id) + "_" + 
                      std::to_string(timestamp) + "_" +
                      std::to_string(rand()) + "_" +
                      std::to_string(attempt);
    
    // Simple hash function (FNV-1a variant)
    unsigned long long hash = 14695981039346656037ULL;
    for (char c : data) {
        hash ^= (unsigned char)c;
        hash *= 1099511628211ULL;
    }
    
    // Encode to alphanumeric: base36 encoding
    static const char base36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string code;
    
    // First part: encoded timestamp (last 6 digits in base36 - ~4 chars)
    unsigned long long ts_part = timestamp % 2176782336ULL; // 36^6
    for (int i = 0; i < 4; ++i) {
        code += base36[ts_part % 36];
        ts_part /= 36;
    }
    
    // Second part: encoded owner_id (3 chars)
    unsigned long long owner_part = owner_id;
    for (int i = 0; i < 3; ++i) {
        code += base36[(owner_part + hash) % 36];
        owner_part /= 36;
        hash /= 7;
    }
    
    // Third part: encoded file_id (3 chars)
    unsigned long long file_part = file_id;
    for (int i = 0; i < 3; ++i) {
        code += base36[(file_part + hash) % 36];
        file_part /= 36;
        hash /= 7;
    }
    
    // Fourth part: hash remainder (6 chars for uniqueness)
    for (int i = 0; i < 6; ++i) {
        code += base36[hash % 36];
        hash /= 36;
    }
    
    return code; // Total: 16 characters
}

std::string DBManager::generateShareCode(long long file_id, std::string owner_username, int max_uses) {
    // Lấy owner_id
    std::string ownerQuery = "SELECT user_id FROM USERS WHERE username = '" + owner_username + "'";
    if (mysql_query(conn, ownerQuery.c_str())) {
        std::cerr << "[DB] generateShareCode: Failed to get owner_id" << std::endl;
        return "";
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return "";
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return "";
    }
    long long owner_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    // Kiểm tra file có thuộc về owner không
    std::string checkQuery = "SELECT file_id FROM FILES WHERE file_id = " + std::to_string(file_id) + 
                            " AND owner_id = " + std::to_string(owner_id) + " AND is_deleted = FALSE";
    if (mysql_query(conn, checkQuery.c_str())) {
        std::cerr << "[DB] generateShareCode: Failed to verify ownership" << std::endl;
        return "";
    }
    
    result = mysql_store_result(conn);
    if (!result || mysql_num_rows(result) == 0) {
        if (result) mysql_free_result(result);
        std::cerr << "[DB] generateShareCode: File not owned by user" << std::endl;
        return "";
    }
    mysql_free_result(result);
    
    // Generate unique code with retry on collision
    const int MAX_ATTEMPTS = 5;
    std::string code;
    
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        code = generateUniqueCode(owner_id, file_id, attempt);
        
        // Try to insert - will fail if code already exists (UNIQUE constraint)
        std::string insertQuery = "INSERT INTO SHARE_CODES (share_code, file_id, owner_id, max_uses) VALUES ('" +
                                 code + "', " + std::to_string(file_id) + ", " + std::to_string(owner_id) + ", " +
                                 std::to_string(max_uses) + ")";
        
        if (mysql_query(conn, insertQuery.c_str()) == 0) {
            // Success
            std::cout << "[DB] Generated share code: " << code << " for file_id: " << file_id 
                      << " (attempt: " << (attempt + 1) << ")" << std::endl;
            return code;
        }
        
        // Check if error is duplicate key
        unsigned int err = mysql_errno(conn);
        if (err == 1062) { // ER_DUP_ENTRY
            std::cerr << "[DB] generateShareCode: Code collision, retrying... (attempt " << (attempt + 1) << ")" << std::endl;
            // Add small delay and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * (attempt + 1)));
            continue;
        } else {
            std::cerr << "[DB] generateShareCode: Insert failed: " << mysql_error(conn) << std::endl;
            return "";
        }
    }
    
    std::cerr << "[DB] generateShareCode: Failed after " << MAX_ATTEMPTS << " attempts" << std::endl;
    return "";
}

long long DBManager::redeemShareCode(std::string share_code, std::string username) {
    // Lấy thông tin mã share
    std::string query = "SELECT sc.code_id, sc.file_id, sc.owner_id, sc.max_uses, sc.current_uses, "
                       "sc.expires_at, sc.is_active FROM SHARE_CODES sc "
                       "WHERE sc.share_code = '" + share_code + "'";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] redeemShareCode: Query failed" << std::endl;
        return -1;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return -1;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        std::cerr << "[DB] redeemShareCode: Code not found" << std::endl;
        return -1;
    }
    
    long long code_id = std::stoll(row[0]);
    long long file_id = std::stoll(row[1]);
    long long owner_id = std::stoll(row[2]);
    int max_uses = row[3] ? std::stoi(row[3]) : 0;
    int current_uses = row[4] ? std::stoi(row[4]) : 0;
    bool is_active = row[6] ? (std::string(row[6]) == "1") : false;
    mysql_free_result(result);
    
    // Kiểm tra mã có active không
    if (!is_active) {
        std::cerr << "[DB] redeemShareCode: Code is not active" << std::endl;
        return -1;
    }
    
    // Kiểm tra số lần sử dụng (max_uses = 0 nghĩa là không giới hạn)
    if (max_uses > 0 && current_uses >= max_uses) {
        std::cerr << "[DB] redeemShareCode: Code has reached max uses" << std::endl;
        return -1;
    }
    
    // Lấy user_id của người redeem
    std::string userQuery = "SELECT user_id FROM USERS WHERE username = '" + username + "'";
    if (mysql_query(conn, userQuery.c_str())) {
        return -1;
    }
    
    result = mysql_store_result(conn);
    if (!result) return -1;
    
    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return -1;
    }
    long long user_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    // Không cho phép redeem file của chính mình
    if (user_id == owner_id) {
        std::cerr << "[DB] redeemShareCode: Cannot redeem own file" << std::endl;
        return -1;
    }
    
    // Kiểm tra xem đã share chưa
    std::string checkShareQuery = "SELECT shared_id FROM SHAREDFILES WHERE file_id = " + std::to_string(file_id) +
                                 " AND user_id = " + std::to_string(user_id);
    if (mysql_query(conn, checkShareQuery.c_str())) {
        return -1;
    }
    
    result = mysql_store_result(conn);
    if (result && mysql_num_rows(result) > 0) {
        mysql_free_result(result);
        std::cerr << "[DB] redeemShareCode: Already shared with this user" << std::endl;
        return file_id; // Đã share rồi, trả về file_id
    }
    if (result) mysql_free_result(result);
    
    // Share file cho user (permission_id = 1 = VIEW)
    std::string shareQuery = "INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES (" +
                            std::to_string(file_id) + ", " + std::to_string(user_id) + ", 1)";
    if (mysql_query(conn, shareQuery.c_str())) {
        std::cerr << "[DB] redeemShareCode: Failed to share: " << mysql_error(conn) << std::endl;
        return -1;
    }
    
    // Tăng current_uses
    std::string updateQuery = "UPDATE SHARE_CODES SET current_uses = current_uses + 1 WHERE code_id = " + std::to_string(code_id);
    mysql_query(conn, updateQuery.c_str());
    
    std::cout << "[DB] Redeemed share code: " << share_code << " for user: " << username << std::endl;
    return file_id;
}

std::vector<ShareInfo> DBManager::getMyShares(std::string username) {
    std::vector<ShareInfo> shares;
    
    std::string query = "SELECT sf.shared_id, sf.file_id, f.name, f.is_folder, u2.username as shared_with, "
                       "p.name as permission, sf.shared_at "
                       "FROM SHAREDFILES sf "
                       "JOIN FILES f ON sf.file_id = f.file_id "
                       "JOIN USERS u1 ON f.owner_id = u1.user_id "
                       "JOIN USERS u2 ON sf.user_id = u2.user_id "
                       "JOIN PERMISSIONS p ON sf.permission_id = p.permission_id "
                       "WHERE u1.username = '" + username + "' AND f.is_deleted = FALSE "
                       "ORDER BY sf.shared_at DESC";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] getMyShares: Query failed: " << mysql_error(conn) << std::endl;
        return shares;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return shares;
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        ShareInfo info;
        info.shared_id = std::stoll(row[0]);
        info.file_id = std::stoll(row[1]);
        info.filename = row[2] ? row[2] : "";
        info.is_folder = row[3] ? (std::string(row[3]) == "1") : false;
        info.shared_with_username = row[4] ? row[4] : "";
        info.permission = row[5] ? row[5] : "";
        info.shared_at = row[6] ? row[6] : "";
        shares.push_back(info);
    }
    
    mysql_free_result(result);
    std::cout << "[DB] getMyShares: Found " << shares.size() << " shares for " << username << std::endl;
    return shares;
}

bool DBManager::revokeShare(long long file_id, std::string owner_username, std::string target_username) {
    // Xác nhận owner
    std::string ownerQuery = "SELECT user_id FROM USERS WHERE username = '" + owner_username + "'";
    if (mysql_query(conn, ownerQuery.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    long long owner_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    // Xác nhận target user
    std::string targetQuery = "SELECT user_id FROM USERS WHERE username = '" + target_username + "'";
    if (mysql_query(conn, targetQuery.c_str())) {
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) return false;
    
    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    long long target_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    // Kiểm tra file có thuộc về owner không
    std::string checkQuery = "SELECT file_id FROM FILES WHERE file_id = " + std::to_string(file_id) +
                            " AND owner_id = " + std::to_string(owner_id);
    if (mysql_query(conn, checkQuery.c_str())) {
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result || mysql_num_rows(result) == 0) {
        if (result) mysql_free_result(result);
        std::cerr << "[DB] revokeShare: Not file owner" << std::endl;
        return false;
    }
    mysql_free_result(result);
    
    // Xóa share
    std::string deleteQuery = "DELETE FROM SHAREDFILES WHERE file_id = " + std::to_string(file_id) +
                             " AND user_id = " + std::to_string(target_id);
    if (mysql_query(conn, deleteQuery.c_str())) {
        std::cerr << "[DB] revokeShare: Delete failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    if (mysql_affected_rows(conn) == 0) {
        std::cerr << "[DB] revokeShare: No share found to revoke" << std::endl;
        return false;
    }
    
    std::cout << "[DB] Revoked share for file_id: " << file_id << " from user: " << target_username << std::endl;
    return true;
}

std::vector<ShareCodeInfo> DBManager::getMyShareCodes(std::string username) {
    std::vector<ShareCodeInfo> codes;
    
    std::string query = "SELECT sc.code_id, sc.share_code, sc.file_id, f.name, f.is_folder, "
                       "sc.max_uses, sc.current_uses, sc.expires_at, sc.is_active, sc.created_at "
                       "FROM SHARE_CODES sc "
                       "JOIN FILES f ON sc.file_id = f.file_id "
                       "JOIN USERS u ON sc.owner_id = u.user_id "
                       "WHERE u.username = '" + username + "' AND sc.is_active = TRUE "
                       "ORDER BY sc.created_at DESC";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] getMyShareCodes: Query failed: " << mysql_error(conn) << std::endl;
        return codes;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return codes;
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        ShareCodeInfo info;
        info.code_id = std::stoll(row[0]);
        info.share_code = row[1] ? row[1] : "";
        info.file_id = std::stoll(row[2]);
        info.filename = row[3] ? row[3] : "";
        info.is_folder = row[4] ? (std::string(row[4]) == "1") : false;
        info.max_uses = row[5] ? std::stoi(row[5]) : 0;
        info.current_uses = row[6] ? std::stoi(row[6]) : 0;
        info.expires_at = row[7] ? row[7] : "";
        info.is_active = row[8] ? (std::string(row[8]) == "1") : false;
        info.created_at = row[9] ? row[9] : "";
        codes.push_back(info);
    }
    
    mysql_free_result(result);
    return codes;
}

bool DBManager::deleteShareCode(std::string share_code, std::string owner_username) {
    // Lấy owner_id
    std::string ownerQuery = "SELECT user_id FROM USERS WHERE username = '" + owner_username + "'";
    if (mysql_query(conn, ownerQuery.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return false;
    }
    long long owner_id = std::stoll(row[0]);
    mysql_free_result(result);
    
    // Xóa mã (hoặc deactivate)
    std::string deleteQuery = "UPDATE SHARE_CODES SET is_active = FALSE WHERE share_code = '" + share_code +
                             "' AND owner_id = " + std::to_string(owner_id);
    if (mysql_query(conn, deleteQuery.c_str())) {
        std::cerr << "[DB] deleteShareCode: Failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    return mysql_affected_rows(conn) > 0;
}

FileRecordEx DBManager::getFileById(long long file_id) {
    FileRecordEx record;
    record.file_id = -1;
    
    std::string query = "SELECT f.file_id, f.owner_id, f.parent_id, f.name, f.is_folder, f.size_bytes, u.username "
                       "FROM FILES f JOIN USERS u ON f.owner_id = u.user_id "
                       "WHERE f.file_id = " + std::to_string(file_id) + " AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) {
        return record;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return record;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        record.file_id = std::stoll(row[0]);
        record.owner_id = std::stoll(row[1]);
        record.parent_id = row[2] ? std::stoll(row[2]) : 0;
        record.name = row[3] ? row[3] : "";
        record.is_folder = row[4] ? (std::string(row[4]) == "1") : false;
        record.size = row[5] ? std::stoll(row[5]) : 0;
        record.owner = row[6] ? row[6] : "";
    }
    
    mysql_free_result(result);
    return record;
}
// ===== GUEST MODE FUNCTIONS =====

bool DBManager::guestRedeemShareCode(std::string share_code, long long &out_file_id, 
                                     std::string &out_filename, bool &out_is_folder, 
                                     std::string &out_owner, long long &out_size) {
    // Lấy thông tin share code
    std::string query = "SELECT sc.code_id, sc.file_id, sc.max_uses, sc.current_uses, sc.is_active, sc.expires_at, "
                       "f.name, f.is_folder, f.size_bytes, u.username "
                       "FROM SHARE_CODES sc "
                       "JOIN FILES f ON sc.file_id = f.file_id "
                       "JOIN USERS u ON sc.owner_id = u.user_id "
                       "WHERE sc.share_code = '" + share_code + "' AND f.is_deleted = FALSE";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] guestRedeemShareCode: Query failed" << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return false;
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        std::cerr << "[DB] guestRedeemShareCode: Code not found" << std::endl;
        return false;
    }
    
    long long code_id = std::stoll(row[0]);
    out_file_id = std::stoll(row[1]);
    int max_uses = std::stoi(row[2]);
    int current_uses = std::stoi(row[3]);
    bool is_active = (std::string(row[4]) == "1");
    std::string expires_at = row[5] ? row[5] : "";
    out_filename = row[6] ? row[6] : "";
    out_is_folder = (std::string(row[7]) == "1");
    out_size = row[8] ? std::stoll(row[8]) : 0;
    out_owner = row[9] ? row[9] : "";
    
    mysql_free_result(result);
    
    // Kiểm tra code còn hoạt động không
    if (!is_active) {
        std::cerr << "[DB] guestRedeemShareCode: Code is inactive" << std::endl;
        return false;
    }
    
    // Kiểm tra số lần sử dụng (max_uses = 0 nghĩa là unlimited)
    if (max_uses > 0 && current_uses >= max_uses) {
        std::cerr << "[DB] guestRedeemShareCode: Max uses reached" << std::endl;
        return false;
    }
    
    // Kiểm tra hết hạn
    if (!expires_at.empty()) {
        std::string checkExpiry = "SELECT NOW() > '" + expires_at + "'";
        if (mysql_query(conn, checkExpiry.c_str()) == 0) {
            MYSQL_RES* expResult = mysql_store_result(conn);
            if (expResult) {
                MYSQL_ROW expRow = mysql_fetch_row(expResult);
                if (expRow && std::string(expRow[0]) == "1") {
                    mysql_free_result(expResult);
                    std::cerr << "[DB] guestRedeemShareCode: Code expired" << std::endl;
                    return false;
                }
                mysql_free_result(expResult);
            }
        }
    }
    
    // Tăng current_uses
    std::string updateQuery = "UPDATE SHARE_CODES SET current_uses = current_uses + 1 WHERE code_id = " + 
                             std::to_string(code_id);
    mysql_query(conn, updateQuery.c_str());
    
    std::cout << "[DB] Guest redeemed code: " << share_code << " for file: " << out_filename << std::endl;
    return true;
}

std::vector<FileRecordEx> DBManager::guestListFolder(long long folder_id) {
    std::vector<FileRecordEx> files;
    
    std::string query = "SELECT f.file_id, f.owner_id, f.parent_id, f.name, f.is_folder, f.size_bytes, u.username "
                       "FROM FILES f JOIN USERS u ON f.owner_id = u.user_id "
                       "WHERE f.parent_id = " + std::to_string(folder_id) + " AND f.is_deleted = FALSE "
                       "ORDER BY f.is_folder DESC, f.name ASC";
    
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "[DB] guestListFolder: Query failed" << std::endl;
        return files;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return files;
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        FileRecordEx record;
        record.file_id = std::stoll(row[0]);
        record.owner_id = std::stoll(row[1]);
        record.parent_id = row[2] ? std::stoll(row[2]) : 0;
        record.name = row[3] ? row[3] : "";
        record.is_folder = (std::string(row[4]) == "1");
        record.size = row[5] ? std::stoll(row[5]) : 0;
        record.owner = row[6] ? row[6] : "";
        files.push_back(record);
    }
    
    mysql_free_result(result);
    return files;
}