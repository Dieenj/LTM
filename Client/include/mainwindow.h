#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include "network_manager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Login page
    void onConnectBtnClicked();
    void onLoginBtnClicked();
    void handleLoginSuccess();

    // Dashboard page
    void onRefreshClicked();
    void onUploadClicked();
    void onDownloadClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onLogoutClicked();
    void onTabChanged(int index);
    
    // Folder navigation
    void showContextMenu(const QPoint &pos);
    void onFolderDoubleClicked(int row, int column);
    void onBackButtonClicked();

    // Network responses
    void handleFileList(QString data);
    void handleUploadProgress(QString msg);
    void handleDownloadComplete(QString filename);
    void handleShareResult(bool success, QString msg);
    void handleDeleteResult(bool success, QString msg);
    void handleLogout();

private:
    void setupUI();
    QWidget* createLoginPage();
    QWidget* createDashboardPage();

    // UI Components
    QStackedWidget *stackedWidget;
    
    // Login page
    QLineEdit *hostInput;
    QLineEdit *userInput;
    QLineEdit *passInput;
    
    // Dashboard page
    QTabWidget *tabWidget;
    QTableWidget *fileTable;
    QTableWidget *sharedFileTable;
    QPushButton *backButton;
    QLabel *pathLabel;
    
    // Current navigation state
    long long currentFolderId;
    QString currentFolderPath;
    
    // Network manager
    NetworkManager *netManager;
};

#endif // MAINWINDOW_H