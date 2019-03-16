#include "server.h"
#include "server_thread_worker.h"

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
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
        qDebug() << "Listening to port " << port << "...";
        emit restartGame(maxClients);
        //qDebug() << "Start game";
        
    }
}

// This function is called by QTcpServer when a new connection is available. 
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection
    qDebug() << socketDescriptor << " Connecting...";

    if (currentPlayer_ == maxClients) {
        qDebug() << socketDescriptor << " Max players reached";
        return;
    }

    emit addPlayer();

    // Every new connection will be run in a newly created thread
    QThread *thread = new QThread(this);
    auto worker = new ThreadWorker(socketDescriptor, currentPlayer_);
    currentPlayer_++;
    
    worker->moveToThread(thread);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(this, &MyServer::sendGameState, worker, &ThreadWorker::sendGameState);
    connect(this, &MyServer::closeConnections, worker, &ThreadWorker::closeConnection);
    connect(worker, &ThreadWorker::connectionLost, this, &MyServer::connectionLost);
    connect(worker, &ThreadWorker::command, this, &MyServer::command);
    auto conn = connect(this, &MyServer::startThreadWorker, worker, &ThreadWorker::start);

    thread->start();
    emit startThreadWorker();
    disconnect(conn);
}

void MyServer::connectionLost(int player)
{
    emit closeConnections();
    if (currentPlayer_ != 0) {

        emit restartGame(maxClients);
        currentPlayer_ = 0;
    }
}
