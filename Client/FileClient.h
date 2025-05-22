#ifndef FILE_CLIENT_H
#define FILE_CLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include "Common.h"

class QListWidget;
class FileClient : public QWidget {
    Q_OBJECT
public:
    explicit FileClient(QWidget *parent = nullptr);

private slots:
    void connectToMetadataNode();
    void uploadFile();
    void readMetadataResponse();

private:
    void setupUi();
    void uploadChunks(const QString &filename, int numChunks, QString currentServer);

    QTcpSocket *socket;
    QString currentFile; // Store file path for upload
    QListWidget *fileList; // For future download functionality
};

#endif // FILE_CLIENT_H
