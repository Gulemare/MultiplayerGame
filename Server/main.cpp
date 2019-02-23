#include <QtCore/QCoreApplication>
#include "server.h"

Q_DECLARE_METATYPE(std::string)

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::string>();

    QCoreApplication a(argc, argv);
    // Make a server and starts it
    MyServer server;
    server.startServer();

    return a.exec();
}
