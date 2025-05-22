#include <QCoreApplication>
#include "ChunkServer.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QList<ChunkServer*> servers;
    for (int i = 1; i <= NUM_CHUNK_SERVERS; ++i) {
        servers << new ChunkServer(i);
    }
    return app.exec();
}
