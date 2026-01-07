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

-- Tạo các folder và file mẫu
INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES
    -- Root folders cho admin
    (1, NULL, 'Documents', TRUE, 0),
    (1, NULL, 'Images', TRUE, 0),
    (1, NULL, 'Videos', TRUE, 0),
    (1, NULL, 'Projects', TRUE, 0),
    (1, NULL, 'Downloads', TRUE, 0),
    
    -- Sub-folders trong Documents
    (1, 1, 'Reports', TRUE, 0),
    (1, 1, 'Presentations', TRUE, 0),
    (1, 1, 'Research', TRUE, 0),
    
    -- Sub-folders trong Projects
    (1, 4, 'WebApps', TRUE, 0),
    (1, 4, 'MobileApps', TRUE, 0),
    (1, 4, 'Scripts', TRUE, 0),
    
    -- Files trong Documents/Reports
    (1, 6, 'Bao_cao_Do_an.pdf', FALSE, 2048576),
    (1, 6, 'Monthly_Report.docx', FALSE, 1024000),
    
    -- Files trong Images
    (1, 2, 'Hinh_anh_demo.png', FALSE, 512000),
    (1, 2, 'Screenshot_2025.jpg', FALSE, 780000),
    
    -- Files trong Projects/WebApps
    (1, 9, 'portfolio-website.zip', FALSE, 5242880),
    (1, 9, 'ecommerce-app.tar.gz', FALSE, 8388608),
    
    -- Root folders cho guest
    (2, NULL, 'MyFiles', TRUE, 0),
    (2, NULL, 'Shared', TRUE, 0),
    
    -- Files cho guest
    (2, 19, 'Source_Code.zip', FALSE, 102400),
    (2, 19, 'notes.txt', FALSE, 4096);

-- Chia sẻ file từ admin cho guest
INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES
    (13, 2, 1), -- Share "Bao_cao_Do_an.pdf" với guest (VIEW permission)
    (15, 2, 1), -- Share "Hinh_anh_demo.png" với guest (VIEW permission)
    (17, 2, 1); -- Share "portfolio-website.zip" với guest (VIEW permission)

-- Đánh dấu star
INSERT INTO STARS (user_id, file_id) VALUES
    (1, 13), -- admin star Bao_cao_Do_an.pdf
    (1, 17); -- admin star portfolio-website.zip
