#include "FileClient.h"
#include "NoiseHandler.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDataStream>
#include <QListWidget>

FileClient::FileClient(QWidget *parent) : QWidget(parent) {
    socket = new QTcpSocket(this);
    fileList = new QListWidget(this);
    setupUi();
    connect(socket, &QTcpSocket::readyRead, this, &FileClient::readMetadataResponse);
}

void FileClient::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *connectBtn = new QPushButton("Connect to Metadata Node", this);
    QPushButton *uploadBtn = new QPushButton("Upload File", this);

    layout->addWidget(fileList);
    layout->addWidget(connectBtn);
    layout->addWidget(uploadBtn);

    connect(connectBtn, &QPushButton::clicked, this, &FileClient::connectToMetadataNode);
    connect(uploadBtn, &QPushButton::clicked, this, &FileClient::uploadFile);
}

void FileClient::connectToMetadataNode() {
    socket->connectToHost("127.0.0.1", METADATA_PORT);
}

void FileClient::uploadFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select File");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QString filename = QFileInfo(filePath).fileName();
            QDataStream out(socket);
            out << QString("UPLOAD") << filename << file.size() << QString("txt");
            file.close();
            currentFile = filePath;
        }
    }
}

void FileClient::readMetadataResponse() {
    QDataStream in(socket);
    QString command;
    in >> command;

    if (command == "CHUNK_INFO") {
        QString filename;
        int numChunks;
        QString firstServer;
        in >> filename >> numChunks >> firstServer;
        socket->disconnectFromHost();
        uploadChunks(filename, numChunks, firstServer);
    } else {
        QString message;
        in >> message;
        QMessageBox::warning(this, "Error", message);
        socket->disconnectFromHost();
    }
}

void FileClient::uploadChunks(const QString &filename, int numChunks, QString currentServer) {
    QFile file(currentFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot open file");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    NoiseHandler noiseHandler;
    for (int i = 0; i < numChunks; ++i) {
        QString chunkId = QString("%1_chunk%2").arg(filename).arg(i);
        QByteArray chunkData = fileData.mid(i * CHUNK_SIZE, CHUNK_SIZE);

        // Encode with Reed-Solomon
        QByteArray encodedData = noiseHandler.encodeReedSolomon(chunkData);
        // Add 1% noise
        QByteArray noisyData = noiseHandler.addNoise(encodedData);

        QTcpSocket chunkSocket;
        QString ip = currentServer.split(":").first();
        int port = currentServer.split(":").last().toInt();
        chunkSocket.connectToHost(ip, port);
        if (!chunkSocket.waitForConnected(5000)) {
            QMessageBox::warning(this, "Error", QString("Cannot connect to chunk server %1").arg(currentServer));
            return;
        }

        // Send chunk
        QDataStream out(&chunkSocket);
        out << QString("STORE_CHUNK") << chunkId << noisyData;
        chunkSocket.waitForReadyRead(5000);

        // Read response
        QDataStream in(&chunkSocket);
        QString command, receivedChunkId, nextServer;
        in >> command >> receivedChunkId >> nextServer;
        if (command == "CORRUPT") {
            QMessageBox::warning(this, "Error", QString("Chunk %1 is corrupt and cannot be recovered").arg(chunkId));
            chunkSocket.disconnectFromHost();
            return;
        }
        if (command != "NEXT_SERVER") {
            QMessageBox::warning(this, "Error", "Invalid response from chunk server");
            chunkSocket.disconnectFromHost();
            return;
        }
        chunkSocket.disconnectFromHost();
        currentServer = nextServer;
        if (nextServer.isEmpty() && i < numChunks - 1) {
            QMessageBox::warning(this, "Error", "No more chunk servers available");
            return;
        }
    }
    QMessageBox::information(this, "Success", "File uploaded successfully");
}
