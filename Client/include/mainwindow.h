#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QProgressDialog>
#include <QHBoxLayout>
#include <QStack>
#include "network_manager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectBtnClicked();
    void onLoginBtnClicked();
    void onRegisterBtnClicked();
    void handleLoginSuccess();
    void handleRegisterSuccess(QString msg);
    void handleRegisterFailed(QString msg);

    void onRefreshClicked();
    void onUploadClicked();
    void onUploadFileClicked();
    void onUploadFolderClicked();
    void onDownloadClicked();
    void onDownloadFolderClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onRenameClicked();
    void onLogoutClicked();
    void onTabChanged(int index);
    
    void showContextMenu(const QPoint &pos);
    void onFolderDoubleClicked(int row, int column);
    void onBackButtonClicked();
    void onBreadcrumbClicked();
    
    // Share code slots
    void onGenerateShareCodeClicked();
    void onRedeemShareCodeClicked();
    void onRevokeShareClicked();
    void onDeleteShareCodeClicked();
    void onRefreshMySharesClicked();
    void handleShareCodeGenerated(bool success, const QString &code, const QString &msg);
    void handleShareCodeRedeemed(bool success, long long file_id, const QString &filename, bool is_folder, const QString &owner);
    void handleMySharesReceived(const QList<ShareInfoClient> &shares);
    void handleShareRevoked(bool success, const QString &msg);
    void handleMyShareCodesReceived(const QList<ShareCodeInfoClient> &codes);
    void handleShareCodeDeleted(bool success, const QString &msg);
    
    // Guest mode slots
    void onGuestAccessClicked();
    void onGuestRedeemCode();
    void onGuestDownloadClicked();
    void onGuestFileDoubleClicked(int row, int column);
    void handleGuestRedeemResult(bool success, long long file_id, const QString &filename, 
                                 bool is_folder, const QString &owner, long long size);
    void handleGuestFolderList(const QList<GuestFileInfo> &files);

    void handleFileList(QString data);
    void handleUploadStarted(QString filename);
    void handleUploadProgress(QString msg);
    void handleDownloadStarted(QString filename);
    void handleDownloadComplete(QString filename);
    void handleShareResult(bool success, QString msg);
    void handleDeleteResult(bool success, QString msg);
    void handleRenameResult(bool success, QString msg);
    void handleLogout();

private:
    void setupUI();
    QWidget* createLoginPage();
    QWidget* createDashboardPage();
    QWidget* createGuestPage();
    QString formatFileSize(long long bytes);

    QStackedWidget *stackedWidget;
    
    QLineEdit *hostInput;
    QLineEdit *userInput;
    QLineEdit *passInput;
    
    QTabWidget *tabWidget;
    QTableWidget *fileTable;
    QTableWidget *sharedFileTable;
    QTableWidget *mySharesTable;      // Tab quản lý file đã share
    QTableWidget *shareCodesTable;    // Tab quản lý mã share
    QPushButton *backButton;
    QLabel *pathLabel;
    QHBoxLayout *breadcrumbLayout;
    
    // Guest mode widgets
    QTableWidget *guestFileTable;
    QLineEdit *guestShareCodeInput;
    QLabel *guestInfoLabel;
    bool isGuestMode;
    long long guestCurrentFileId;
    QString guestCurrentFileName;
    bool guestCurrentIsFolder;
    
    long long currentFolderId;
    QString currentFolderPath;
    QString currentUsername;
    
    QStack<long long> folderHistory;
    
    NetworkManager *netManager;
    
    QProgressDialog *uploadProgressDialog;
    QProgressDialog *downloadProgressDialog;
};

#endif