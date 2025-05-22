#include "ChunkServer.h"
#include "NoiseHandler.h"
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QTextStream>

ChunkServer::ChunkServer(int nodeId, QObject *parent) : QObject(parent), nodeId(nodeId) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &ChunkServer::handleNewConnection);
    int port = FIRST_CHUNK_PORT + nodeId - 1;
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Chunk Server" << nodeId << "could not start on port" << port;
    } else {
        qDebug() << "Chunk Server" << nodeId << "started on port" << port;
    }
    // Initialize chunk servers
    for (int i = 1; i <= NUM_CHUNK_SERVERS; ++i) {
        chunkServers << QString("%1:%2").arg(CHUNK_SERVER_IP).arg(FIRST_CHUNK_PORT + i - 1);
    }
    // Create chunk directory
    chunkDir = QString("D:/CHUNK-%1").arg(nodeId);
    QDir().mkpath(chunkDir);
    // Initialize DFS order
    dfsOrder << 1 << 2 << 4 << 8 << 9 << 5 << 10 << 11 << 3 << 6 << 12 << 13 << 7 << 14 << 15;
}

void ChunkServer::handleNewConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, [this, client]() {
        QDataStream in(client);
        QString command, chunkId;
        in >> command >> chunkId;

        if (command == "STORE_CHUNK") {
            QByteArray noisyData;
            in >> noisyData;

            // Decode with Reed-Solomon
            NoiseHandler noiseHandler;
            bool isCorrupt;
            int errorCount;
            QByteArray chunkData = noiseHandler.decodeReedSolomon(noisyData, isCorrupt, errorCount);
            qDebug() << "Chunk" << chunkId << "received with" << errorCount << "errors";

            if (isCorrupt) {
                // Mark chunk as corrupt
                QFile corruptFile(QString("%1/%2.corrupt").arg(chunkDir).arg(chunkId));
                corruptFile.open(QIODevice::WriteOnly);
                corruptFile.close();
                QDataStream out(client);
                out << QString("CORRUPT") << chunkId << "";
                client->disconnectFromHost();
                return;
            }

            // Store chunk
            QFile file(QString("%1/%2").arg(chunkDir).arg(chunkId));
            if (file.open(QIODevice::WriteOnly)) {
                file.write(chunkData);
                file.close();
            }

            // Find next server using DFS order
            QString nextServer = "";
            int currentIndex = dfsOrder.indexOf(nodeId);
            if (currentIndex + 1 < dfsOrder.size()) {
                int nextNodeId = dfsOrder[currentIndex + 1];
                nextServer = chunkServers[nextNodeId - 1];
            }

            // Store next server address
            QFile metaFile(QString("%1/%2.meta").arg(chunkDir).arg(chunkId));
            if (metaFile.open(QIODevice::WriteOnly)) {
                QTextStream out(&metaFile);
                out << nextServer;
                metaFile.close();
            }

            // Send next server address to client
            QDataStream out(client);
            out << QString("NEXT_SERVER") << chunkId << nextServer;
        }
        client->disconnectFromHost();
    });
}
