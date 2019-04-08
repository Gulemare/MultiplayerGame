#pragma once
#include <QWebSocketServer>
#include <QThread>
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
            auto state = game_.getState();
            auto cmd = state.add_commands();
            cmd->CopyFrom(command);
            emit gameStateUpdated(state);
        }
    }

signals:
    void started();
    void gameStateUpdated(GameState state);

private:
    game::Game game_;
};

class MyServer : public QWebSocketServer
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
    void closeConnections();
    void command(Command command, int player);

private:
    QThread gameThread_;
    int currentPlayer_ = 0;
    const int maxClients = 2;

private slots:
    void connectionLost(int player);
    void onNewConnection();
};
