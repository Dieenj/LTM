# File Management Application

Ứng dụng quản lý file Client-Server đa luồng với giao diện Qt6 và cơ sở dữ liệu MySQL.

## Tổng Quan

### Kiến trúc
- **Server:** C++ với MySQL, epoll-based I/O multiplexing
- **Client:** Qt6 GUI application
- **Protocol:** FTP-inspired custom protocol
- **Thread Model:** Fixed Worker Pool (4 threads) + Dedicated I/O Threads

### Công nghệ
- C++17, Qt6, MySQL 8.0+, OpenSSL, CMake 3.10+

---

## Tính Năng

- ✅ Upload/Download file với chunk-based ACK (64KB chunks, ACK mỗi 1MB)
- ✅ Chia sẻ file với permission system (VIEW/EDIT/DELETE/SHARE)
- ✅ Quản lý thư mục với breadcrumb navigation
- ✅ Đăng ký tài khoản và quản lý quota
- ✅ GUI 2 tab: "My Files" (Home/) và "Shared with Me" (Shared/)
- ✅ Load balancing và thread monitoring

---

## Yêu Cầu Hệ Thống

```bash
# Ubuntu/Debian
sudo apt install cmake g++ libmysqlclient-dev libssl-dev mysql-server qt6-base-dev

# Fedora/RHEL
sudo dnf install cmake gcc-c++ mysql-devel openssl-devel qt6-qtbase-devel
```

---

## Hướng Dẫn Cài Đặt

### 1. Setup Database
```bash
cd database
chmod +x setup_database.sh
./setup_database.sh
```

Nhập MySQL root password khi được hỏi. Script tự động tạo:
- Database `file_management`
- Tables: USERS, FILES, SHAREDFILES, PERMISSIONS, STARS
- Demo users: admin/123456 (2GB), guest/guest (1GB), dien/123456 (10GB)
- File `Server/Core/include/db_config.h` với thông tin kết nối MySQL

**⚠️ Lưu ý:** File `db_config.h` đã được thêm vào `.gitignore` - không commit file này!

### 2. Cấu hình Storage Path (Optional)
Mặc định file upload lưu tại `Server/storage/`. Để đổi path:

```bash
# Sửa file Server/Core/include/server_config.h
nano Server/Core/include/server_config.h

# Thay đổi dòng:
#define STORAGE_PATH "Server/storage/"
# Thành path mong muốn, ví dụ:
#define STORAGE_PATH "/home/user/fileserver_storage/"
```

Tạo thư mục storage mới:
```bash
mkdir -p /home/user/fileserver_storage/
chmod 755 /home/user/fileserver_storage/
```

### 3. Chạy Server
```bash
./run_server.sh
```

### 4. Chạy Client (terminal mới)
```bash
./run_client.sh
```

---

## Hướng Dẫn Sử Dụng

### Kết nối và Đăng nhập
1. **Connect**: Server IP `127.0.0.1` → Click "Connect Server"
2. **Login**: Dùng tài khoản `admin/123456`
3. **Register** (optional): Click "Register New Account"

### Quản lý File
- **Upload**: Click "Upload" → chọn file
- **Download**: Chọn file → Click "Download"
- **Navigate**: Double-click vào folder, dùng Back button
- **Share**: Right-click file → "Share" → nhập username
- **Delete**: Right-click → "Delete"

### Navigation
- **My Files tab**: Hiển thị "Home/" và subfolder
- **Shared tab**: Hiển thị "Shared/" và file được share
- **Breadcrumb**: Hiển thị path với folder history

---

## Kiến Trúc Hệ Thống

### Thread Model
- **AcceptorThread**: Lắng nghe port 8080, load balancing
- **WorkerThread Pool** (4): Xử lý kết nối với epoll
- **DedicatedThread**: On-demand cho file I/O (max 100)
- **ThreadMonitor**: Giám sát và thống kê

### Protocol Commands
| Command | Description | Response |
|---------|-------------|----------|
| REGISTER \<user\> \<pass\> | Đăng ký | 200/550 |
| USER \<user\> | Đăng nhập | 331 |
| PASS \<pass\> | Xác thực | 230/530 |
| LIST [\<folder\>] | Liệt kê files | 150+data |
| STOR \<name\> \<size\> | Upload | 150/550 |
| RETR \<name\> | Download | 150/550 |
| SHARE \<file\> \<user\> \<perm\> | Share | 200/550 |
| DELE \<id\> | Xóa | 250/550 |
| MKDIR \<name\> | Tạo folder | 257/550 |

### Chunk ACK
- Upload/Download: 64KB chunks, ACK mỗi 1MB
- Timeout: 3s
- Benefits: Phát hiện lỗi sớm, tối ưu performance

---

## Cấu Hình Database

Sau khi chạy `setup_database.sh`, file `Server/Core/include/db_config.h` được tạo tự động:

```cpp
#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "your_password"
#define DB_NAME "file_management"
#define DB_PORT 3306

#endif
```

**Mặc định:**
- **Database name:** `file_management`
- **MySQL user:** `root` (hoặc user bạn chỉ định khi chạy setup)
- **Host:** `localhost`
- **Port:** `3306`

**Demo accounts trong database:**
| Username | Password | Storage Quota |
|----------|----------|---------------|
| admin    | 123456   | 2GB           |
| guest    | guest    | 1GB           |
| dien     | 123456   | 10GB          |

**Để đổi thông tin kết nối:**
1. Sửa file `Server/Core/include/db_config.h`
2. Hoặc chạy lại `./database/setup_database.sh`

**⚠️ Bảo mật:**
- File này chứa password MySQL
- Đã được thêm vào `.gitignore`
- **KHÔNG ĐƯỢC commit** file này lên git

---

## Cấu Hình Storage Path

File upload mặc định lưu tại `Server/storage/`. Để đổi:

**File:** `Server/Core/include/server_config.h`
```cpp
#define STORAGE_PATH "Server/storage/"  // Đổi path này
```

**Lưu ý:**
- Path phải kết thúc bằng `/`
- Đảm bảo thư mục tồn tại và có quyền write
- Rebuild server sau khi đổi: `./run_server.sh`

**Ví dụ:**
```cpp
// Path tuyệt đối
#define STORAGE_PATH "/home/user/my_storage/"

// Path tương đối
#define STORAGE_PATH "../shared_storage/"
```

---

## Cấu Trúc Dự Án

```
.
├── Client/          # Qt6 GUI source
├── Server/          # C++ server source
│   ├── Core/
│   │   ├── include/
│   │   │   └── server_config.h  # Đổi STORAGE_PATH ở đây
│   │   └── src/
│   └── storage/     # Uploaded files (default)
├── Common/          # Protocol.h
├── database/
│   ├── schema.sql
│   └── setup_database.sh
├── run_server.sh
├── run_client.sh
└── README.md
```

---

## Troubleshooting

### Cannot connect to database
```bash
sudo systemctl start mysql
cd database && ./setup_database.sh
```

### Permission denied
```bash
chmod +x run_server.sh run_client.sh database/setup_database.sh
```

### Script cannot execute / CRLF line endings
Nếu gặp lỗi "cannot execute: required file not found" khi chạy script:
```bash
# Fix line endings (CRLF → LF)
sed -i 's/\r$//' run_server.sh run_client.sh test_db.sh
sed -i 's/\r$//' database/setup_database.sh
chmod +x run_server.sh run_client.sh test_db.sh database/setup_database.sh

# Hoặc dùng dos2unix nếu có
dos2unix *.sh database/*.sh
```

### Build fails
```bash
rm -rf build build_client Client/build
./run_server.sh
```

### Server không nhận kết nối
```bash
sudo netstat -tuln | grep 8080
sudo ufw allow 8080/tcp
```

---

## Database Schema

- **USERS**: user_id, username, password_hash, storage_limit_bytes
- **FILES**: file_id, owner_id, parent_id, name, is_folder, size_bytes, is_deleted
- **SHAREDFILES**: file_id, user_id, permission_id
- **PERMISSIONS**: VIEW, EDIT, DELETE, SHARE
- **STARS**: user_id, file_id (starred files)

---

## Testing

```bash
# Test database
./test_db.sh

# Load test (multiple clients)
for i in {1..10}; do ./run_client.sh & done
```

---

## Changelog

### v1.0 (Current)
- ✅ File operations (upload/download/delete/share)
- ✅ User registration with SHA256 password hashing
- ✅ Chunk-based ACK mechanism
- ✅ Breadcrumb navigation (Home/ and Shared/)
- ✅ Folder history with back button
- ✅ Thread monitoring
- ✅ Database auto-setup

### Planned
- [ ] File encryption
- [ ] Resume transfers
- [ ] Search functionality
- [ ] File versioning
- [ ] Trash bin
