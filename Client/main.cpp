#include <QApplication>
#include "client.h"

Q_DECLARE_METATYPE(std::string)

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::string>();

    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName(Client::tr("Game Client"));
    Client client;
    client.show();
    return app.exec();
}
