-- File Management System Database Schema

DROP DATABASE IF EXISTS file_management;
CREATE DATABASE file_management CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE file_management;

-- Bảng USERS: Lưu trữ thông tin người dùng
CREATE TABLE USERS (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    storage_limit_bytes BIGINT DEFAULT 1073741824, -- 1GB mặc định
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_username (username)
) ENGINE=InnoDB;

-- Bảng PERMISSIONS: Định nghĩa các loại quyền
CREATE TABLE PERMISSIONS (
    permission_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE
) ENGINE=InnoDB;

-- Thêm các quyền cơ bản
INSERT INTO PERMISSIONS (name) VALUES 
    ('VIEW'),
    ('EDIT'),
    ('DELETE'),
    ('SHARE');

-- Bảng FILES: Lưu trữ metadata của file/folder
CREATE TABLE FILES (
    file_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    owner_id BIGINT NOT NULL,
    parent_id BIGINT NULL, -- NULL = thư mục gốc
    name VARCHAR(255) NOT NULL,
    is_folder BOOLEAN DEFAULT FALSE,
    size_bytes BIGINT DEFAULT 0,
    is_deleted BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    
    FOREIGN KEY (owner_id) REFERENCES USERS(user_id) ON DELETE CASCADE,
    FOREIGN KEY (parent_id) REFERENCES FILES(file_id) ON DELETE CASCADE,
    
    INDEX idx_owner (owner_id),
    INDEX idx_parent (parent_id),
    INDEX idx_name (name),
    INDEX idx_deleted (is_deleted)
) ENGINE=InnoDB;

-- Bảng SHAREDFILES: Lưu trữ quyền chia sẻ
CREATE TABLE SHAREDFILES (
    shared_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    file_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    permission_id INT NOT NULL,
    shared_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (file_id) REFERENCES FILES(file_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES USERS(user_id) ON DELETE CASCADE,
    FOREIGN KEY (permission_id) REFERENCES PERMISSIONS(permission_id),
    
    UNIQUE KEY unique_share (file_id, user_id, permission_id),
    INDEX idx_file (file_id),
    INDEX idx_user (user_id)
) ENGINE=InnoDB;

-- Bảng STARS: Lưu trữ file được đánh dấu quan trọng
CREATE TABLE STARS (
    user_id BIGINT NOT NULL,
    file_id BIGINT NOT NULL,
    starred_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (user_id, file_id),
    FOREIGN KEY (user_id) REFERENCES USERS(user_id) ON DELETE CASCADE,
    FOREIGN KEY (file_id) REFERENCES FILES(file_id) ON DELETE CASCADE
) ENGINE=InnoDB;

-- Tạo user mẫu (password: 123456 - đã hash bằng SHA256)
INSERT INTO USERS (username, password_hash, storage_limit_bytes) VALUES
    ('admin', SHA2('123456', 256), 2147483648), -- 2GB
    ('guest', SHA2('guest', 256), 1073741824);   -- 1GB

-- Tạo một số file mẫu
INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES
    (1, NULL, 'Documents', TRUE, 0),
    (1, NULL, 'Images', TRUE, 0),
    (1, 1, 'Bao_cao_Do_an.pdf', FALSE, 2048576),
    (1, 2, 'Hinh_anh_demo.png', FALSE, 512000),
    (2, NULL, 'Source_Code.zip', FALSE, 102400);

-- Chia sẻ file từ admin cho guest
INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES
    (3, 2, 1); -- Share "Bao_cao_Do_an.pdf" với guest (VIEW permission)

-- Đánh dấu star
INSERT INTO STARS (user_id, file_id) VALUES
    (1, 3);
