#include <QWebSocket>
#include "server.h"
#include "server_thread_worker.h"


MyServer::MyServer(QObject *parent) :
    QWebSocketServer("Game server", QWebSocketServer::NonSecureMode)
{
    qDebug() << "Server started on thread: " << QThread::currentThreadId();
    GameWorker *game = new GameWorker;
    game->moveToThread(&gameThread_);
    connect(this, &MyServer::restartGame, game, &GameWorker::restart);
    connect(this, &MyServer::addPlayer, game, &GameWorker::addPlayer);
    connect(this, &MyServer::command, game, &GameWorker::consumeCommand);
    connect(game, &GameWorker::gameStateUpdated, this, &MyServer::sendGameState);
    gameThread_.start();
}

MyServer::~MyServer()
{
    gameThread_.quit();
    gameThread_.wait();
}

void MyServer::startServer()
{
    int port = 1234;

    if (!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        connect(this, &QWebSocketServer::newConnection, this, &MyServer::onNewConnection);
        qDebug() << "Listening to port " << port << "...";
        emit restartGame(maxClients);
        //qDebug() << "Start game";
        
    }
}

void MyServer::onNewConnection()
{
    QWebSocket *pSocket = this->nextPendingConnection();

    // We have a new connection
    qDebug() << " Connecting...";
    if (currentPlayer_ == maxClients) {
        return;
    }

    //connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processTextMessage);
    const auto playerNumber = currentPlayer_;
    currentPlayer_++;
    connect(pSocket, &QWebSocket::binaryMessageReceived, [this, playerNumber](const QByteArray& data) {
        Command cmd;
        if (!cmd.ParseFromArray(data.data(), data.size()))
            return;

        emit command(cmd, playerNumber);
    });

    connect(pSocket, &QWebSocket::disconnected, [this, playerNumber]{
        qDebug() << playerNumber << ": disconnected";
        this->connectionLost(playerNumber);
    });

    connect(this, &MyServer::closeConnections, [pSocket]() { pSocket->close(); });
    connect(this, &MyServer::sendGameState, [this, playerNumber, pSocket](GameState state) {
        state.set_player(playerNumber);
        QByteArray msg(state.ByteSize(), Qt::Uninitialized);
        state.SerializeToArray(msg.data(), msg.size());
        pSocket->sendBinaryMessage(msg);
    });

    emit addPlayer();
}

void MyServer::connectionLost(int player)
{
    emit closeConnections();
    if (currentPlayer_ != 0) {

        emit restartGame(maxClients);
        currentPlayer_ = 0;
    }
}
