#include "../../include/request_handler.h"
#include "../../include/db_manager.h"
#include "../../../../Common/Protocol.h"

std::string AuthHandler::handleUser(int fd, ClientSession& session, const std::string& username) {
    session.username = username;
    return "331 Password required\n";
}

std::string AuthHandler::handlePass(int fd, ClientSession& session, const std::string& password) {
    if (session.username.empty()) {
        return std::string(CODE_FAIL) + " Login with USER first\n";
    }

    if (DBManager::getInstance().checkUser(session.username, password)) {
        session.isAuthenticated = true;
        return std::string(CODE_LOGIN_SUCCESS) + " Login successful\n";
    } else {
        return std::string(CODE_LOGIN_FAIL) + " Login failed\n";
    }
}

std::string AuthHandler::handleRegister(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return std::string(CODE_FAIL) + " Username and password cannot be empty\n";
    }
    
    if (username.length() < 3) {
        return std::string(CODE_FAIL) + " Username must be at least 3 characters\n";
    }
    
    if (password.length() < 4) {
        return std::string(CODE_FAIL) + " Password must be at least 4 characters\n";
    }
    
    if (DBManager::getInstance().registerUser(username, password)) {
        return std::string(CODE_OK) + " Registration successful\n";
    } else {
        return std::string(CODE_FAIL) + " Username already exists\n";
    }
}