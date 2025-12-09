# HƯỚNG DẪN DEMO GUI - UPLOAD/DOWNLOAD

## Chuẩn bị

### 1. Khởi động Server
```bash
cd /home/cuong/DuAnPMCSF/File_Management_App
./run_server.sh
```

Server sẽ lắng nghe trên **port 8080**

### 2. Khởi động Client (cần môi trường GUI)
Mở terminal mới:
```bash
cd /home/cuong/DuAnPMCSF/File_Management_App
./run_client.sh
```

## Demo các chức năng

### BƯỚC 1: Kết nối Server
1. Trong giao diện Client, giữ nguyên **Server IP: 127.0.0.1**
2. Click nút **"1. Connect Server"**
3. Popup hiển thị "Connected to Server!"

### BƯỚC 2: Đăng nhập
Sử dụng một trong các tài khoản sau:
- **Username:** `admin` / **Password:** `123456`
- **Username:** `guest` / **Password:** `guest`

1. Nhập username và password
2. Click nút **"2. Login"**
3. Màn hình chuyển sang Dashboard

### BƯỚC 3: Xem danh sách file
- Dashboard tự động hiển thị danh sách file (mock data)
- Bảng gồm 3 cột: Filename | Size | Owner
- Click **"Refresh"** để cập nhật danh sách

### BƯỚC 4: Upload File
1. Click nút **"Upload File"**
2. Chọn file từ máy tính (bất kỳ file nào)
3. Client sẽ:
   - Kiểm tra quota với server
   - Gửi file lên server
   - Hiển thị thông báo thành công
4. File được lưu tại: `/home/cuong/DuAnPMCSF/File_Management_App/Server/storage/<filename>`

### BƯỚC 5: Download File
1. Chọn 1 file trong bảng (click vào hàng)
2. Click nút **"Download Selected"**
3. File được tải về thư mục: `~/Downloads/<filename>`

## Kiểm tra kết quả

### Kiểm tra file đã upload
```bash
ls -lh /home/cuong/DuAnPMCSF/File_Management_App/Server/storage/
```

### Kiểm tra file đã download
```bash
ls -lh ~/Downloads/
```

## Lưu ý

### Mock Data hiện tại:
Server đang trả về 3 file giả lập:
- `Bao_cao_Do_an.pdf` (2MB) - owner: admin
- `Hinh_anh_demo.png` (500KB) - owner: admin
- `Source_Code.zip` (100KB) - owner: guest

**Các file này chưa tồn tại thực tế**, chỉ hiển thị trên GUI.

### Upload thật:
- Khi bạn upload file, nó được lưu thực tế vào `Server/storage/`
- Tuy nhiên danh sách file vẫn hiển thị mock data (chưa kết nối database)

### Download:
- Chỉ download được các file trong mock data (chưa tồn tại thực tế)
- Để download được file đã upload, cần kết nối database thật

## Các vấn đề đã biết

1. **Danh sách file không cập nhật sau upload**: Do chưa có database thật
2. **Download file mock sẽ lỗi**: Vì file chưa tồn tại trên server
3. **Quota check luôn pass**: Do dùng giá trị hard-code

## Để hoàn thiện

Cần implement:
1. Database MySQL thật (bảng USERS, FILES)
2. Lưu thông tin file vào DB khi upload
3. Lấy danh sách file từ DB thay vì mock data
4. Kiểm tra quota thật từ database
