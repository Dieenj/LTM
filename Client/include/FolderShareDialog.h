#ifndef FOLDERSHAREDIALOG_H
#define FOLDERSHAREDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "network_manager.h"

class FolderShareDialog : public QDialog {
    Q_OBJECT

public:
    explicit FolderShareDialog(long long folderId, 
                              const QString &folderName,
                              const QString &targetUser,
                              NetworkManager *netManager,
                              QWidget *parent = nullptr);
    ~FolderShareDialog();
    
    void setLocalFolderPath(const QString &basePath);

private slots:
    void onFolderStructureReceived(long long folder_id, QList<FileNodeInfo> structure);
    void onShareInitiated(const QString &session_id, int total_files, const QList<FileNodeInfo> &files);
    void onFileUploaded(int completed, int total);
    void onShareCompleted(const QString &session_id);
    void onShareFailed(const QString &error);
    void onProgressUpdated(int percentage, const QString &status);
    void onCancelClicked();
    void startUpload();

private:
    void setupUI();
    void loadFolderPreview();
    void uploadNextFile();
    QString formatSize(qint64 bytes);
    
    long long m_folderId;
    QString m_folderName;
    QString m_targetUser;
    QString m_sessionId;
    QString m_localFolderBasePath;
    
    NetworkManager *m_netManager;
    
    QLabel *m_titleLabel;
    QTreeWidget *m_previewTree;
    QLabel *m_statsLabel;
    QPushButton *m_shareButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    QList<FileNodeInfo> m_filesToUpload;
    int m_currentFileIndex;
    int m_totalFiles;
    int m_completedFiles;
    qint64 m_totalSize;
    bool m_uploadInProgress;
};

#endif