#include <QApplication>
#include "FileClient.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    FileClient client;
    client.show();
    return app.exec();
}
