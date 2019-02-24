#pragma once

#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include "game.h"

/*! Worker for game, lives in separate thread */
class GameWorker : public QObject
{
    Q_OBJECT

public slots:

    void start() {
        qDebug() << "Game started on thread: " << QThread::currentThreadId();
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &GameWorker::gameTick, Qt::ConnectionType::QueuedConnection);
        timer->start(15);
    }

    void gameTick() {
        game_.tick();
        emit gameStateUpdated(game_.getState());
    }

    void addPlayer(QPoint pos, const QString& name) {
        game_.addPlayer(pos, name.toStdString().data());
        qDebug() << "Player added";
    }

    void removePlayer(const int id) {
        game_.removePlayer(id);
    }

    void commandPlayerMove(int playerId, float x, float y) {
        game_.updatePlayerMovePath(playerId, { { x, y } });
    }

signals:
    void gameStateUpdated(const std::string& state);

private:
    Game game_;
};

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = 0);
    ~MyServer();
    void startServer();

signals:
    void startGame();
    void addPlayer(QPoint pos, const QString& name);
    void removePlayer(const int id);
    void sendGameState(const std::string& state);
    void startThreadWorker();

    void commandPlayerMove(int playerId, float x, float y);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QThread gameThread_;

};
