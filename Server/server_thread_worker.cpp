#include "server_thread_worker.h"
#include <chrono>

void ThreadWorker::start() {
    socket_ = new QTcpSocket();
    if (!socket_->setSocketDescriptor(socketDescriptor_))
    {
        emit error(socket_->error());
        return;
    }

    in_.setDevice(socket_);
    in_.setVersion(QDataStream::Qt_5_10);

    connect(socket_, &QIODevice::readyRead, this, &ThreadWorker::readyRead, Qt::DirectConnection);
    connect(socket_, &QTcpSocket::disconnected, this, &ThreadWorker::disconnected);
    qDebug() << socketDescriptor_ << " Client connected";
}

void ThreadWorker::readyRead() {
    // get the commands from client
    in_.startTransaction();
    QByteArray data;
    in_ >> data;
    if (!in_.commitTransaction())
        return;

    Command cmd;
    if (!cmd.ParseFromArray(data.data(), data.size()))
        return;

    emit command(cmd, player_);

    /*
    qDebug() << QString("%1(%2): (%3, %4)")
        .arg(socketDescriptor_)
        .arg((int)QThread::currentThreadId())
        .arg(cmd.target().x())
        .arg(cmd.target().y());
    emit moveCommand(socketDescriptor_, cmd.target().x(), cmd.target().y());
    */
}

void ThreadWorker::disconnected() {
    qDebug() << socketDescriptor_ << " Disconnected";
    emit connectionLost(socketDescriptor_);
    socket_->deleteLater();
    
    QThread::currentThread()->exit(0);
}

void filterGameState(GameState& state, int player) {

    state.set_player(player);
}

void ThreadWorker::sendGameState(GameState state) {
    filterGameState(state, player_);

    QByteArray msg(state.ByteSize(), Qt::Uninitialized);
    state.SerializeToArray(msg.data(), msg.size());

    // send game state to client
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << msg;
    socket_->write(block);

    //////////// CONNECTION STATISTICS /////////////////////////
    static auto last = std::chrono::steady_clock().now();
    static auto accumulator = 0;
    static auto packetCounter = 0;
    auto now = std::chrono::steady_clock().now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    last = now;
    packetCounter++;
    accumulator += elapsed;
    if (accumulator >= 1000) {
        //qDebug() << QString("%1 send states in sec: %2").arg(socketDescriptor_).arg(packetCounter);
        accumulator = 0;
        packetCounter = 0;
    }
    /////////////////////////////////////////////////////////////
}

void ThreadWorker::closeConnection()
{
    socket_->close();
}

