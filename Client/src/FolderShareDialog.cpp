#include "FolderShareDialog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>

FolderShareDialog::FolderShareDialog(long long folderId,
                                    const QString &folderName,
                                    const QString &targetUser,
                                    NetworkManager *netManager,
                                    QWidget *parent)
    : QDialog(parent),
      m_folderId(folderId),
      m_folderName(folderName),
      m_targetUser(targetUser),
      m_netManager(netManager),
      m_currentFileIndex(0),
      m_totalFiles(0),
      m_completedFiles(0),
      m_totalSize(0),
      m_uploadInProgress(false)
{
    setupUI();
    
    connect(m_netManager, &NetworkManager::folderStructureReceived,
            this, &FolderShareDialog::onFolderStructureReceived);
    connect(m_netManager, &NetworkManager::folderShareInitiated,
            this, &FolderShareDialog::onShareInitiated);
    connect(m_netManager, &NetworkManager::folderFileUploaded,
            this, &FolderShareDialog::onFileUploaded);
    connect(m_netManager, &NetworkManager::folderShareCompleted,
            this, &FolderShareDialog::onShareCompleted);
    connect(m_netManager, &NetworkManager::folderShareFailed,
            this, &FolderShareDialog::onShareFailed);
    connect(m_netManager, &NetworkManager::folderShareProgress,
            this, &FolderShareDialog::onProgressUpdated);
    
    loadFolderPreview();
}

FolderShareDialog::~FolderShareDialog() {}

void FolderShareDialog::setLocalFolderPath(const QString &basePath) {
    m_localFolderBasePath = basePath;
}

void FolderShareDialog::setupUI() {
    setWindowTitle("Share Folder");
    setMinimumSize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_titleLabel = new QLabel(QString("Share folder '%1' with user '%2'")
                              .arg(m_folderName, m_targetUser));
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(m_titleLabel);
    
    m_previewTree = new QTreeWidget();
    m_previewTree->setHeaderLabels({"Name", "Type", "Size"});
    m_previewTree->setColumnWidth(0, 300);
    m_previewTree->setColumnWidth(1, 100);
    m_previewTree->setAlternatingRowColors(true);
    m_previewTree->setSelectionMode(QAbstractItemView::NoSelection);
    mainLayout->addWidget(m_previewTree);
    
    m_statsLabel = new QLabel("Loading folder structure...");
    m_statsLabel->setStyleSheet("color: #666; padding: 10px;");
    mainLayout->addWidget(m_statsLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(true);
    mainLayout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel();
    m_statusLabel->setVisible(false);
    m_statusLabel->setStyleSheet("color: #0066cc; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, &QPushButton::clicked, this, &FolderShareDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    m_shareButton = new QPushButton("Share");
    m_shareButton->setEnabled(false);
    m_shareButton->setStyleSheet(
        "QPushButton { background-color: #0066cc; color: white; "
        "padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0052a3; }"
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    connect(m_shareButton, &QPushButton::clicked, this, &FolderShareDialog::startUpload);
    buttonLayout->addWidget(m_shareButton);
    
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

void FolderShareDialog::loadFolderPreview() {
    m_statsLabel->setText("Loading folder structure from server...");
    m_netManager->getFolderStructure(m_folderId);
}

void FolderShareDialog::onFolderStructureReceived(long long folder_id, QList<FileNodeInfo> structure) {
    if (folder_id != m_folderId) return;
    
    m_previewTree->clear();
    
    QMap<long long, QTreeWidgetItem*> itemMap;
    
    QTreeWidgetItem *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, m_folderName);
    rootItem->setText(1, "Folder");
    rootItem->setExpanded(true);
    m_previewTree->addTopLevelItem(rootItem);
    itemMap[m_folderId] = rootItem;
    
    int fileCount = 0;
    int folderCount = 0;
    m_totalSize = 0;
    
    for (const FileNodeInfo &item : structure) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem();
        treeItem->setText(0, item.name);
        
        if (item.is_folder) {
            treeItem->setText(1, "Folder");
            treeItem->setExpanded(true);
            folderCount++;
        } else {
            treeItem->setText(1, "File");
            treeItem->setText(2, formatSize(item.size));
            fileCount++;
            m_totalSize += item.size;
        }
        
        if (itemMap.contains(item.parent_id)) {
            itemMap[item.parent_id]->addChild(treeItem);
        } else {
            rootItem->addChild(treeItem);
        }
        
        itemMap[item.file_id] = treeItem;
    }
    
    m_statsLabel->setText(QString("📁 %1 folders  |  📄 %2 files  |  💾 %3")
                          .arg(folderCount)
                          .arg(fileCount)
                          .arg(formatSize(m_totalSize)));
    
    m_totalFiles = fileCount;
    m_shareButton->setEnabled(fileCount > 0);
}

void FolderShareDialog::startUpload() {
    m_shareButton->setEnabled(false);
    m_previewTree->setEnabled(false);
    
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    
    m_statusLabel->setVisible(true);
    m_statusLabel->setText("Initiating folder share...");
    
    m_uploadInProgress = true;
    
    m_netManager->shareFolderRequest(m_folderId, m_targetUser);
}

void FolderShareDialog::onShareInitiated(const QString &session_id, 
                                        int total_files, 
                                        const QList<FileNodeInfo> &files) {
    m_sessionId = session_id;
    m_filesToUpload = files;
    m_currentFileIndex = 0;
    m_completedFiles = 0;
    
    m_statusLabel->setText(QString("Session created. Uploading %1 files...").arg(total_files));
    
    m_localFolderBasePath = QDir::homePath() + "/Downloads/" + m_folderName;
    
    uploadNextFile();
}

void FolderShareDialog::uploadNextFile() {
    if (m_currentFileIndex >= m_filesToUpload.size()) {
        return;
    }
    
    const FileNodeInfo &fileInfo = m_filesToUpload[m_currentFileIndex];
    
    m_statusLabel->setText(QString("Uploading: %1 (%2/%3)")
                          .arg(fileInfo.name)
                          .arg(m_currentFileIndex + 1)
                          .arg(m_filesToUpload.size()));
    
    m_netManager->uploadFolderFile(m_sessionId, fileInfo, m_localFolderBasePath);
    
    m_currentFileIndex++;
}

void FolderShareDialog::onFileUploaded(int completed, int total) {
    m_completedFiles = completed;
    
    int percentage = (total > 0) ? (completed * 100 / total) : 0;
    m_progressBar->setValue(percentage);
    
    m_statusLabel->setText(QString("Uploading files: %1/%2 (%3%)")
                          .arg(completed)
                          .arg(total)
                          .arg(percentage));
    
    if (completed < total) {
        uploadNextFile();
    }
}

void FolderShareDialog::onShareCompleted(const QString &session_id) {
    if (session_id != m_sessionId) return;
    
    m_progressBar->setValue(100);
    m_statusLabel->setText("✅ Folder shared successfully!");
    m_statusLabel->setStyleSheet("color: #00aa00; padding: 5px; font-weight: bold;");
    
    m_cancelButton->setText("Close");
    m_uploadInProgress = false;
    
    QMessageBox::information(this, "Success",
        QString("Folder '%1' shared successfully!\n%2 files transferred.")
        .arg(m_folderName)
        .arg(m_totalFiles));
}

void FolderShareDialog::onShareFailed(const QString &error) {
    m_progressBar->setVisible(false);
    m_statusLabel->setText("❌ Share failed: " + error);
    m_statusLabel->setStyleSheet("color: #cc0000; padding: 5px;");
    
    m_shareButton->setEnabled(true);
    m_previewTree->setEnabled(true);
    m_uploadInProgress = false;
    
    QMessageBox::critical(this, "Error", "Failed to share folder:\n" + error);
}

void FolderShareDialog::onProgressUpdated(int percentage, const QString &status) {
    m_progressBar->setValue(percentage);
    m_statusLabel->setText(status);
}

void FolderShareDialog::onCancelClicked() {
    if (m_uploadInProgress && m_completedFiles > 0 && m_completedFiles < m_totalFiles) {
        auto reply = QMessageBox::question(this, "Cancel Share",
            "Upload in progress. Are you sure you want to cancel?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
        
        if (!m_sessionId.isEmpty()) {
            m_netManager->cancelFolderShare(m_sessionId);
        }
    }
    
    reject();
}

QString FolderShareDialog::formatSize(qint64 bytes) {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " bytes";
    }
}