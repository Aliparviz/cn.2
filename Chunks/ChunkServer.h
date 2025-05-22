#ifndef CHUNK_SERVER_H
#define CHUNK_SERVER_H

#include <QObject>
#include <QTcpServer>
#include "Common.h"

class ChunkServer : public QObject {
    Q_OBJECT
public:
    explicit ChunkServer(int nodeId, QObject *parent = nullptr);

private slots:
    void handleNewConnection();

private:
    QTcpServer *server;
    int nodeId; // Node ID (1 to 15)
    QString chunkDir; // Directory for chunks (e.g., D:/CHUNK-1)
    QStringList chunkServers; // List of chunk server addresses
    QList<int> dfsOrder; // DFS traversal order
};

#endif // CHUNK_SERVER_H
