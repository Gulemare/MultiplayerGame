#pragma once

#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include "../Common/protocol.pb.h"
#include "../Game/game.h"

/*! Worker for game, lives in separate thread */
class GameWorker : public QObject
{
    Q_OBJECT

public slots:

    void restart(int playerCount) {
        qDebug() << "Game started on thread: " << QThread::currentThreadId();
        game_.restart(playerCount);
    }

    void addPlayer() {
        game_.addPlayer();
        qDebug() << "Player added";

        if (game_.started()) {
            emit gameStateUpdated(game_.getState());
            emit started();
        }
    }

    void consumeCommand(Command command, int player) {
        if (!game_.started())
            return;

        if (game_.consumeCommand(player, command)) {
            qDebug() << player << ": command";
            emit gameStateUpdated(game_.getState());
        }
    }

signals:
    void started();
    void gameStateUpdated(GameState state);

private:
    game::Game game_;
};

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = 0);
    ~MyServer();
    void startServer();

signals:
    void restartGame(int playerCount);
    void addPlayer();
    void sendGameState(GameState state);
    void startThreadWorker();
    void closeConnections();
    void command(Command command, int player);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QThread gameThread_;
    int currentPlayer_ = 0;
    const int maxClients = 2;

private slots:
    void connectionLost(int player);
};
