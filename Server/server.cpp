#include "server.h"
#include "server_thread_worker.h"

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
{
    qDebug() << "Server started on thread: " << QThread::currentThreadId();
    GameWorker *game = new GameWorker;
    game->moveToThread(&gameThread);
    connect(this, &MyServer::startGame, game, &GameWorker::start);
    connect(this, &MyServer::addPlayer, game, &GameWorker::addPlayer);
    connect(this, &MyServer::removePlayer, game, &GameWorker::removePlayer);
    connect(this, &MyServer::commandPlayerMove, game, &GameWorker::commandPlayerMove);
    connect(game, &GameWorker::gameStateUpdated, this, &MyServer::sendGameState, Qt::QueuedConnection);
    gameThread.start();
}

MyServer::~MyServer()
{
    gameThread.quit();
    gameThread.wait();
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
        qDebug() << "Start game";
        emit startGame();
    }
}

// This function is called by QTcpServer when a new connection is available. 
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection
    qDebug() << socketDescriptor << " Connecting...";

    emit addPlayer(QPoint{0, 0}, QString::number(socketDescriptor));

    // Every new connection will be run in a newly created thread
    QThread *thread = new QThread(this);
    auto worker = new ThreadWorker(socketDescriptor);
    worker->moveToThread(thread);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(this, &MyServer::sendGameState, worker, &ThreadWorker::sendGameState);
    connect(worker, &ThreadWorker::connectionLost, this, &MyServer::removePlayer);
    connect(worker, &ThreadWorker::moveCommand, this, &MyServer::commandPlayerMove);
    auto conn = connect(this, &MyServer::startThreadWorker, worker, &ThreadWorker::start);

    thread->start();
    emit startThreadWorker();
    disconnect(conn);
}
