#include <QtCore/QCoreApplication>
#include "server.h"

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(GameState)
Q_DECLARE_METATYPE(Command)

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::string>();
    qRegisterMetaType<GameState>();
    qRegisterMetaType<Command>();

    QCoreApplication a(argc, argv);
    // Make a server and starts it
    MyServer server;
    server.startServer();

    return a.exec();
}
