#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

// Protocol Commands
#define CMD_USER "USER"
#define CMD_PASS "PASS"
#define CMD_LIST "LIST"
#define CMD_LISTSHARED "LISTSHARED"
#define CMD_UPLOAD_CHECK "SITE QUOTA_CHECK"
#define CMD_UPLOAD "STOR"
#define CMD_DOWNLOAD "RETR"
#define CMD_SHARE "SHARE"
#define CMD_DELETE "DELETE"

// ===== NEW: FOLDER SHARE COMMANDS =====
#define CMD_GET_FOLDER_STRUCTURE "GET_FOLDER_STRUCTURE"
#define CMD_SHARE_FOLDER "SHARE_FOLDER"
#define CMD_UPLOAD_FOLDER_FILE "UPLOAD_FOLDER_FILE"
#define CMD_CHECK_SHARE_PROGRESS "CHECK_SHARE_PROGRESS"
#define CMD_CANCEL_FOLDER_SHARE "CANCEL_FOLDER_SHARE"

// Response Codes
#define CODE_OK "200"
#define CODE_FAIL "500"
#define CODE_LOGIN_SUCCESS "230"
#define CODE_LOGIN_FAIL "530"
#define CODE_DATA_OPEN "150"
#define CODE_TRANSFER_COMPLETE "226"

// ===== NEW: FOLDER SHARE STRUCTURES =====
struct FileNodeInfo {
    long long file_id;
    QString name;
    bool is_folder;
    long long size;
    long long parent_id;
    QString relative_path;
};

struct FolderShareSessionInfo {
    QString session_id;
    long long folder_id;
    int total_files;
    int completed_files;
    QList<FileNodeInfo> files_to_upload;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    
    // ===== EXISTING FUNCTIONS - GIỮ NGUYÊN =====
    void connectToServer(const QString &host, quint16 port);
    void login(const QString &user, const QString &pass);
    void requestFileList();
    void requestSharedFileList();
    void uploadFile(const QString &filePath);
    void downloadFile(const QString &filename, const QString &savePath);
    void shareFile(const QString &filename, const QString &targetUser);
    void deleteFile(const QString &filename);
    void logout();

    // ===== NEW: FOLDER SHARE FUNCTIONS =====
    
    // Lấy cấu trúc folder từ server
    void getFolderStructure(long long folder_id);
    
    // Khởi tạo folder share
    void shareFolderRequest(long long folder_id, const QString &targetUser);
    
    // Upload từng file trong folder
    void uploadFolderFile(const QString &session_id, const FileNodeInfo &fileInfo, const QString &localBasePath);
    
    // Check progress
    void checkShareProgress(const QString &session_id);
    
    // Cancel share
    void cancelFolderShare(const QString &session_id);

signals:
    // ===== EXISTING SIGNALS - GIỮ NGUYÊN =====
    void connectionStatus(bool success, QString msg);
    void loginSuccess();
    void loginFailed(QString msg);
    void fileListReceived(QString data);
    void uploadProgress(QString msg);
    void downloadComplete(QString filename);
    void shareResult(bool success, QString msg);
    void deleteResult(bool success, QString msg);
    void logoutSuccess();
    void transferProgress(qint64 current, qint64 total);

    // ===== NEW: FOLDER SHARE SIGNALS =====
    void folderStructureReceived(long long folder_id, QList<FileNodeInfo> structure);
    void folderShareInitiated(const QString &session_id, int total_files, const QList<FileNodeInfo> &files);
    void folderFileUploaded(int completed, int total);
    void folderShareCompleted(const QString &session_id);
    void folderShareFailed(const QString &error);
    void folderShareProgress(int percentage, const QString &status);

private slots:
    void onReadyRead();

private:
    QTcpSocket *socket;
    QString currentHost;
    quint16 currentPort;
    QString currentUsername;
    QString currentPassword;
    
    // ===== NEW: FOLDER SHARE STATE =====
    FolderShareSessionInfo currentFolderShare;
    bool isFolderShareActive = false;
};

#endif // NETWORK_MANAGER_H