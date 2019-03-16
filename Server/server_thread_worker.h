#pragma once

#include <QThread>
#include <QTcpSocket>
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include "../Common/protocol.pb.h"

class ThreadWorker : public QObject
{
    Q_OBJECT

public:
    explicit ThreadWorker(qintptr ID, int player) : socketDescriptor_(ID), player_(player) {}

public slots:
    void start();
    void readyRead();
    void disconnected();
    void sendGameState(GameState state);
    void closeConnection();

signals:
    void error(QTcpSocket::SocketError socketerror);
    void connectionLost(const int sockId);
    void command(Command command, int player_);

private:
    QTcpSocket* socket_;
    QDataStream in_;
    qintptr socketDescriptor_;
    int player_ = 0;
};
