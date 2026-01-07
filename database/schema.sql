-- ====================================
-- FILE MANAGEMENT DATABASE SETUP
-- Complete database initialization
-- ====================================

-- Drop and create database
DROP DATABASE IF EXISTS file_management;
CREATE DATABASE file_management CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE file_management;

-- ====================================
-- TABLE DEFINITIONS
-- ====================================

CREATE TABLE USERS (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    storage_limit_bytes BIGINT DEFAULT 1073741824,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_username (username)
) ENGINE=InnoDB;

CREATE TABLE PERMISSIONS (
    permission_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE
) ENGINE=InnoDB;

INSERT INTO PERMISSIONS (name) VALUES 
    ('VIEW'),
    ('EDIT'),
    ('DELETE'),
    ('SHARE');

CREATE TABLE FILES (
    file_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    owner_id BIGINT NOT NULL,
    parent_id BIGINT NULL,
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

CREATE TABLE STARS (
    user_id BIGINT NOT NULL,
    file_id BIGINT NOT NULL,
    starred_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (user_id, file_id),
    FOREIGN KEY (user_id) REFERENCES USERS(user_id) ON DELETE CASCADE,
    FOREIGN KEY (file_id) REFERENCES FILES(file_id) ON DELETE CASCADE
) ENGINE=InnoDB;

-- ====================================
-- INITIAL DATA
-- ====================================

-- Default users
INSERT INTO USERS (username, password_hash, storage_limit_bytes) VALUES
    ('admin', SHA2('123456', 256), 2147483648), -- 2GB
    ('guest', SHA2('guest', 256), 1073741824),   -- 1GB
    ('dien', SHA2('123456', 256), 10737418240);  -- 10GB

-- Demo folder structure for admin user
INSERT INTO FILES (owner_id, parent_id, name, is_folder, size_bytes) VALUES
    (1, NULL, 'Documents', TRUE, 0),
    (1, NULL, 'Images', TRUE, 0),
    (1, NULL, 'Videos', TRUE, 0),
    (1, NULL, 'Projects', TRUE, 0),
    (1, NULL, 'Downloads', TRUE, 0),
    
    (1, 1, 'Reports', TRUE, 0),
    (1, 1, 'Presentations', TRUE, 0),
    (1, 1, 'Research', TRUE, 0),
    
    (1, 4, 'WebApps', TRUE, 0),
    (1, 4, 'MobileApps', TRUE, 0),
    (1, 4, 'Scripts', TRUE, 0),
    
    (1, 6, 'Bao_cao_Do_an.pdf', FALSE, 2048576),
    (1, 6, 'Monthly_Report.docx', FALSE, 1024000),
    
    (1, 2, 'Hinh_anh_demo.png', FALSE, 512000),
    (1, 2, 'Screenshot_2025.jpg', FALSE, 780000),
    
    (1, 9, 'portfolio-website.zip', FALSE, 5242880),
    (1, 9, 'ecommerce-app.tar.gz', FALSE, 8388608),
    
    (2, NULL, 'MyFiles', TRUE, 0),
    (2, NULL, 'Shared', TRUE, 0),
    
    (2, 19, 'Source_Code.zip', FALSE, 102400),
    (2, 19, 'notes.txt', FALSE, 4096);

-- Demo file shares
INSERT INTO SHAREDFILES (file_id, user_id, permission_id) VALUES
    (13, 2, 1),
    (15, 2, 1),
    (17, 2, 1);

-- Demo starred files
INSERT INTO STARS (user_id, file_id) VALUES
    (1, 13),
    (1, 17);

-- ====================================
-- CLEANUP OPERATIONS (Optional)
-- Use these commands when you need to reset
-- ====================================

-- To clean all data but keep schema:
-- DELETE FROM STARS;
-- DELETE FROM SHAREDFILES;
-- ALTER TABLE SHAREDFILES AUTO_INCREMENT = 1;
-- DELETE FROM FILES;
-- ALTER TABLE FILES AUTO_INCREMENT = 1;
-- DELETE FROM USERS WHERE username NOT IN ('admin', 'dien');

-- ====================================
-- VERIFICATION
-- ====================================

SELECT '✓ Database setup complete!' as Status;
SELECT COUNT(*) as user_count FROM USERS;
SELECT COUNT(*) as file_count FROM FILES;
SELECT COUNT(*) as share_count FROM SHAREDFILES;
