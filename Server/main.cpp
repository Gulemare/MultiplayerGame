#include <QtCore/QCoreApplication>
#include "server.h"

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(GameState)

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::string>();
    qRegisterMetaType<GameState>();

    QCoreApplication a(argc, argv);
    // Make a server and starts it
    MyServer server;
    server.startServer();

    return a.exec();
}
