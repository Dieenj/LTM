#include "network_manager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QThread>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
}

void NetworkManager::connectToServer(const QString &host, quint16 port) {
    currentHost = host;
    currentPort = port;
    
    socket->connectToHost(host, port);
    if(socket->waitForConnected(3000)) {
        emit connectionStatus(true, "Connected to Server!");
    } else {
        emit connectionStatus(false, "Connection Failed!");
    }
}

void NetworkManager::login(const QString &user, const QString &pass) {
    if(socket->state() != QAbstractSocket::ConnectedState) {
        emit loginFailed("Not connected to server!");
        return;
    }
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmdUser = QString("%1 %2\n").arg(CMD_USER, user);
    socket->write(cmdUser.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(3000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit loginFailed("Timeout: Server not responding");
        return;
    }
    
    QString userResponse = QString::fromUtf8(socket->readAll()).trimmed();
    
    if (!userResponse.startsWith("331")) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit loginFailed("Unexpected server response: " + userResponse);
        return;
    }
    
    QString cmdPass = QString("%1 %2\n").arg(CMD_PASS, pass);
    socket->write(cmdPass.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(3000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit loginFailed("Timeout: Server not responding to password");
        return;
    }
    
    QString passResponse = QString::fromUtf8(socket->readAll()).trimmed();
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (passResponse.startsWith(CODE_LOGIN_SUCCESS)) {
        currentUsername = user;
        currentPassword = pass;
        emit loginSuccess();
    } else if (passResponse.startsWith(CODE_LOGIN_FAIL)) {
        emit loginFailed("Invalid username or password");
    } else {
        emit loginFailed("Unexpected server response: " + passResponse);
    }
}
void NetworkManager::registerAccount(const QString &user, const QString &pass) {
    if(socket->state() != QAbstractSocket::ConnectedState) {
        emit registerFailed("Not connected to server!");
        return;
    }

    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmdRegister = QString("%1 %2 %3\n").arg(CMD_REGISTER, user, pass);
    socket->write(cmdRegister.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit registerFailed("Timeout: Server not responding");
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith(CODE_OK)) {
        emit registerSuccess("Registration successful! You can now login.");
    } else if (response.startsWith(CODE_FAIL)) {
        QString errorMsg = "Registration failed";
        if (response.contains("already exists")) {
            errorMsg = "Username already exists!";
        } else if (response.contains("invalid")) {
            errorMsg = "Invalid username or password format!";
        } else {
            errorMsg = response;
        }
        emit registerFailed(errorMsg);
    } else {
        emit registerFailed("Unexpected server response: " + response);
    }
}
void NetworkManager::requestFileList(long long parent_id) {
    currentParentId = parent_id;
    QString cmd = QString("%1 %2\n").arg(CMD_LIST).arg(parent_id);
    socket->write(cmd.toUtf8());
    socket->flush();
}

void NetworkManager::requestSharedFileList(long long parent_id) {
    QString cmd;
    if (parent_id < 0) {
        cmd = QString("%1\n").arg(CMD_LISTSHARED);
    } else {
        cmd = QString("%1 %2\n").arg(CMD_LISTSHARED).arg(parent_id);
    }
    socket->write(cmd.toUtf8());
    socket->flush();
}

void NetworkManager::uploadFile(const QString &filePath, long long parent_id) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit uploadProgress("Failed to open file!");
        return;
    }

    QFileInfo fileInfo(filePath);
    QString filename = fileInfo.fileName();
    qint64 filesize = fileInfo.size();
    
    currentParentId = parent_id;

    emit uploadStarted(filename);
    emit transferProgress(0, filesize);

    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    auto handleError = [&](const QString &msg) {
        file.close();
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit uploadProgress(msg);
    };

    QString checkCmd = QString("%1 %2 %3\n").arg(CMD_UPLOAD_CHECK).arg(filename).arg(filesize);
    socket->write(checkCmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        handleError("Timeout: Server not responding to quota check.");
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    if (!response.startsWith(CODE_OK)) {
        handleError("Upload rejected: " + response);
        return;
    }

    QString storCmd = QString("%1 %2 %3 %4\n").arg(CMD_UPLOAD).arg(filename).arg(filesize).arg(parent_id);
    socket->write(storCmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        handleError("Timeout: Server not responding to upload request.");
        return;
    }
    
    response = QString::fromUtf8(socket->readAll()).trimmed();
    if (!response.startsWith(CODE_DATA_OPEN)) {
        handleError("Server rejected data connection: " + response);
        return;
    }

    qint64 totalSent = 0;
    char buffer[65536];
    const qint64 ACK_GROUP_SIZE = 1048576;
    qint64 bytesSinceLastAck = 0;
    int chunkCount = 0;

    emit transferProgress(0, filesize);

    while (!file.atEnd()) {
        if (socket->state() != QAbstractSocket::ConnectedState) {
            handleError("Network disconnected during upload!");
            return;
        }

        qint64 bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead == -1) {
            handleError("Error reading local file.");
            return;
        }

        qint64 bytesWritten = socket->write(buffer, bytesRead);
        if (bytesWritten == -1) {
             handleError("Socket write error.");
             return;
        }
        
        socket->waitForBytesWritten(100);
        totalSent += bytesWritten;
        bytesSinceLastAck += bytesWritten;
        chunkCount++;
        
        emit transferProgress(totalSent, filesize);
        
        if (bytesSinceLastAck >= ACK_GROUP_SIZE) {
            socket->flush();
            if (!socket->waitForReadyRead(3000)) {
                handleError("Timeout waiting for chunk ACK.");
                return;
            }
            QString ack = QString::fromUtf8(socket->readAll()).trimmed();
            if (!ack.startsWith(CODE_CHUNK_ACK)) {
                handleError("Invalid chunk ACK: " + ack);
                return;
            }
            bytesSinceLastAck = 0;
            chunkCount = 0;
        }
    }
    
    socket->flush();
    file.close();

    if (!socket->waitForReadyRead(15000)) {
        handleError("Timeout waiting for server confirmation.");
        return;
    }
    
    response = QString::fromUtf8(socket->readAll()).trimmed();
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith(CODE_TRANSFER_COMPLETE)) {
        emit uploadProgress("Upload successful: " + filename);
        QTimer::singleShot(200, this, [this]() {
            requestFileList(currentParentId);
        });
    } else {
        emit uploadProgress("Upload finished but server reported error: " + response);
    }
}

void NetworkManager::downloadFile(const QString &filename, const QString &savePath) {
    if(socket->state() != QAbstractSocket::ConnectedState) {
        emit downloadComplete("Not connected to server!");
        return;
    }
    
    emit downloadStarted(filename);
    emit transferProgress(0, 1);
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    auto handleDownloadError = [&](const QString &msg, QFile &f) {
        f.close();
        f.remove();
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit downloadComplete(msg);
    };

    QString cmd = QString("%1 %2\n").arg(CMD_DOWNLOAD, filename);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        QFile dummy;
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit downloadComplete("Timeout: Server not responding to download request.");
        return;
    }

    QString response = QString::fromUtf8(socket->readLine()).trimmed();
    
    if (!response.startsWith(CODE_DATA_OPEN)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit downloadComplete("Download failed: " + response);
        return;
    }

    QStringList parts = response.split(' ');
    qint64 filesize = 0;
    if (parts.size() >= 2) {
        filesize = parts[1].toLongLong();
    }
    
    if (filesize == 0) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit downloadComplete("Error: File is empty or invalid response from server");
        return;
    }
    
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit downloadComplete("Cannot save file (Permission denied?): " + savePath);
        return;
    }

    emit transferProgress(0, filesize);

    qint64 totalReceived = 0;
    int retryCount = 0;
    const qint64 ACK_GROUP_SIZE = 1048576;
    qint64 bytesSinceLastAck = 0;

    while (totalReceived < filesize) {
        if (!socket->waitForReadyRead(5000)) {
            if (socket->state() != QAbstractSocket::ConnectedState) {
                handleDownloadError("Network connection lost!", file);
                return;
            }
            retryCount++;
            if (retryCount > 3) {
                 handleDownloadError("Data transfer timeout.", file);
                 return;
            }
            continue;
        }
        
        retryCount = 0;

        QByteArray chunk = socket->readAll();
        qint64 bytesWritten = file.write(chunk);
        
        if (bytesWritten == -1) {
            handleDownloadError("Disk full or write error!", file);
            return;
        }

        totalReceived += bytesWritten;
        bytesSinceLastAck += bytesWritten;
        
        emit transferProgress(totalReceived, filesize);
        
        if (bytesSinceLastAck >= ACK_GROUP_SIZE && totalReceived < filesize) {
            QString ack = QString("%1 Received %2 bytes\n").arg(CODE_CHUNK_ACK).arg(totalReceived);
            socket->write(ack.toUtf8());
            socket->flush();
            bytesSinceLastAck = 0;
        }
    }
    
    file.close();

    if (socket->bytesAvailable() > 0 || socket->waitForReadyRead(2000)) {
        QString finalMsg = QString::fromUtf8(socket->readLine()).trimmed();
    }
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (totalReceived == filesize) {
        emit downloadComplete("File saved successfully to: " + savePath);
    } else {
        emit downloadComplete("Warning: File size mismatch. Downloaded: " + QString::number(totalReceived));
    }
}

void NetworkManager::shareFile(const QString &filename, const QString &targetUser) {
    if(socket->state() != QAbstractSocket::ConnectedState) {
        emit shareResult(false, "Not connected!");
        return;
    }

    QString cmd = QString("SHARE %1 %2\n").arg(filename, targetUser);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (!socket->waitForReadyRead(5000)) {
        emit shareResult(false, "Timeout waiting for share response.");
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith(CODE_OK)) {
        emit shareResult(true, "File shared successfully!");
    } else {
        emit shareResult(false, "Share failed: " + response);
    }
}

void NetworkManager::deleteFile(const QString &filename) {
    if(socket->state() != QAbstractSocket::ConnectedState) {
        emit deleteResult(false, "Not connected!");
        return;
    }

    QString cmd = QString("DELETE %1\n").arg(filename);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (!socket->waitForReadyRead(5000)) {
        emit deleteResult(false, "Timeout waiting for delete response.");
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith(CODE_OK)) {
        emit deleteResult(true, "File deleted successfully!");
    } else {
        emit deleteResult(false, "Delete failed: " + response);
    }
}

void NetworkManager::logout() {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    currentUsername.clear();
    currentPassword.clear();
    emit logoutSuccess();
}

void NetworkManager::onReadyRead() {
    QByteArray allData = socket->readAll();
    QString fullResponse = QString::fromUtf8(allData).trimmed();
    
    if (fullResponse.isEmpty()) return;
    
    QStringList lines = fullResponse.split('\n', Qt::SkipEmptyParts);
    
    QStringList fileListLines;
    bool hasLoginSuccess = false;
    bool hasLoginFailed = false;
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        
        if (trimmed.startsWith(CODE_LOGIN_SUCCESS)) {
            hasLoginSuccess = true;
        } 
        else if (trimmed.startsWith(CODE_LOGIN_FAIL)) {
            hasLoginFailed = true;
        }
        else if (trimmed.startsWith("210")) {
            emit fileListReceived("");
            return;
        }
        else if (trimmed.contains('|')) {
            fileListLines.append(trimmed);
        }
    }
    
    if (hasLoginSuccess) {
        emit loginSuccess();
    }
    if (hasLoginFailed) {
        emit loginFailed("Invalid Username or Password");
    }
    
    if (!fileListLines.isEmpty()) {
        QString fileListData = fileListLines.join("\n");
        emit fileListReceived(fileListData);
    }
}
void NetworkManager::getFolderStructure(long long folder_id) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        emit folderShareFailed("Not connected to server!");
        return;
    }
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmd = QString("%1 %2\n").arg(CMD_GET_FOLDER_STRUCTURE).arg(folder_id);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit folderShareFailed("Timeout getting folder structure");
        return;
    }
    
    QString response;
    while (socket->canReadLine()) {
        response += QString::fromUtf8(socket->readLine());
    }
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QStringList lines = response.split('\n', Qt::SkipEmptyParts);
    
    if (lines.isEmpty() || !lines[0].startsWith("200")) {
        emit folderShareFailed("Failed to get folder structure: " + response);
        return;
    }
    
    QList<FileNodeInfo> structure;
    bool inItems = false;
    
    for (const QString &line : lines) {
        if (line.startsWith("ITEMS:")) {
            inItems = true;
            continue;
        }
        
        if (inItems && line.contains('|')) {
            QStringList parts = line.split('|');
            if (parts.size() >= 5) {
                FileNodeInfo info;
                info.file_id = parts[0].toLongLong();
                info.name = parts[1];
                info.is_folder = (parts[2] == "FOLDER");
                info.size = parts[3].toLongLong();
                info.parent_id = parts[4].toLongLong();
                structure.append(info);
            }
        }
    }
    
    emit folderStructureReceived(folder_id, structure);
}

void NetworkManager::shareFolderRequest(long long folder_id, const QString &targetUser) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        emit folderShareFailed("Not connected to server!");
        return;
    }
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmd = QString("%1 %2 %3\n").arg(CMD_SHARE_FOLDER).arg(folder_id).arg(targetUser);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(10000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit folderShareFailed("Timeout initiating folder share");
        return;
    }
    
    QString response;
    while (socket->canReadLine()) {
        response += QString::fromUtf8(socket->readLine());
    }
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QStringList lines = response.split('\n', Qt::SkipEmptyParts);
    
    if (lines.isEmpty() || !lines[0].startsWith("200")) {
        emit folderShareFailed("Failed to initiate share: " + response);
        return;
    }
    
    QString session_id;
    int total_files = 0;
    QList<FileNodeInfo> files;
    bool inFiles = false;
    
    for (const QString &line : lines) {
        if (line.startsWith("SESSION_ID:")) {
            session_id = line.mid(11).trimmed();
        } else if (line.startsWith("TOTAL_FILES:")) {
            total_files = line.mid(12).trimmed().toInt();
        } else if (line.startsWith("FILES:")) {
            inFiles = true;
            continue;
        }
        
        if (inFiles && line.contains('|')) {
            QStringList parts = line.split('|');
            if (parts.size() >= 4) {
                FileNodeInfo info;
                info.file_id = parts[0].toLongLong();
                info.name = parts[1];
                info.relative_path = parts[2];
                info.size = parts[3].toLongLong();
                info.is_folder = false;
                files.append(info);
            }
        }
    }
    
    if (session_id.isEmpty()) {
        emit folderShareFailed("Invalid session ID from server");
        return;
    }
    
    currentFolderShare.session_id = session_id;
    currentFolderShare.folder_id = folder_id;
    currentFolderShare.total_files = total_files;
    currentFolderShare.completed_files = 0;
    currentFolderShare.files_to_upload = files;
    isFolderShareActive = true;
    
    emit folderShareInitiated(session_id, total_files, files);
}

void NetworkManager::uploadFolderFile(const QString &session_id, 
                                     const FileNodeInfo &fileInfo, 
                                     const QString &localBasePath) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        emit folderShareFailed("Connection lost!");
        return;
    }
    
    QString localFilePath = localBasePath + "/" + fileInfo.relative_path;
    
    QFile file(localFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit folderShareFailed("Cannot open file: " + localFilePath);
        return;
    }
    
    qint64 fileSize = file.size();
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmd = QString("%1 %2 %3 %4\n")
                    .arg(CMD_UPLOAD_FOLDER_FILE)
                    .arg(session_id)
                    .arg(fileInfo.file_id)
                    .arg(fileSize);
    
    socket->write(cmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(5000)) {
        file.close();
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit folderShareFailed("Timeout waiting for upload ACK");
        return;
    }
    
    QString ack = QString::fromUtf8(socket->readLine()).trimmed();
    
    if (!ack.startsWith(CODE_DATA_OPEN)) {
        file.close();
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit folderShareFailed("Server rejected file upload: " + ack);
        return;
    }
    
    qint64 totalSent = 0;
    char buffer[65536];
    
    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer, sizeof(buffer));
        if (bytesRead == -1) {
            file.close();
            connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
            emit folderShareFailed("Error reading file: " + localFilePath);
            return;
        }
        
        qint64 bytesWritten = socket->write(buffer, bytesRead);
        if (bytesWritten == -1) {
            file.close();
            connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
            emit folderShareFailed("Socket write error");
            return;
        }
        
        socket->waitForBytesWritten(100);
        totalSent += bytesWritten;
    }
    
    socket->flush();
    file.close();
    
    if (!socket->waitForReadyRead(10000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        emit folderShareFailed("Timeout waiting for upload confirmation");
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith("202")) {
        QStringList parts = response.split('|');
        int completed = 0;
        int total = 0;
        
        for (const QString &part : parts) {
            if (part.contains("COMPLETED:")) {
                completed = part.split(':')[1].toInt();
            } else if (part.contains("TOTAL:")) {
                total = part.split(':')[1].toInt();
            }
        }
        
        currentFolderShare.completed_files = completed;
        emit folderFileUploaded(completed, total);
        
        int percentage = (total > 0) ? (completed * 100 / total) : 0;
        emit folderShareProgress(percentage, QString("Uploading files: %1/%2").arg(completed).arg(total));
        
    } else if (response.startsWith(CODE_TRANSFER_COMPLETE)) {
        emit folderShareCompleted(session_id);
        emit folderShareProgress(100, "Folder share completed!");
        
        isFolderShareActive = false;
        currentFolderShare = FolderShareSessionInfo();
        
        QTimer::singleShot(500, this, [this]() {
            requestFileList(0);
        });
        
    } else {
        emit folderShareFailed("Upload failed: " + response);
    }
}

void NetworkManager::checkShareProgress(const QString &session_id) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    disconnect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    QString cmd = QString("%1 %2\n").arg(CMD_CHECK_SHARE_PROGRESS).arg(session_id);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    if (!socket->waitForReadyRead(3000)) {
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        return;
    }
    
    QString response = QString::fromUtf8(socket->readAll()).trimmed();
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    if (response.startsWith("200")) {
        QStringList parts = response.split('|');
        int completed = 0;
        int total = 0;
        QString status;
        
        for (const QString &part : parts) {
            if (part.contains("STATUS:")) {
                status = part.split(':')[1];
            } else if (part.contains("COMPLETED:")) {
                completed = part.split(':')[1].toInt();
            } else if (part.contains("TOTAL:")) {
                total = part.split(':')[1].toInt();
            }
        }
        
        int percentage = (total > 0) ? (completed * 100 / total) : 0;
        emit folderShareProgress(percentage, QString("%1: %2/%3").arg(status).arg(completed).arg(total));
    }
}

void NetworkManager::cancelFolderShare(const QString &session_id) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    QString cmd = QString("%1 %2\n").arg(CMD_CANCEL_FOLDER_SHARE).arg(session_id);
    socket->write(cmd.toUtf8());
    socket->flush();
    
    isFolderShareActive = false;
    currentFolderShare = FolderShareSessionInfo();
}