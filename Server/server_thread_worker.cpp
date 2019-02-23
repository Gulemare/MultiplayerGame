#include "server_thread_worker.h"
#include "../Common/protocol.pb.h"

void ThreadWorker::start() {
    socket_ = new QTcpSocket();
    if (!socket_->setSocketDescriptor(this->socketDescriptor_))
    {
        emit error(socket_->error());
        return;
    }

    in.setDevice(socket_);
    in.setVersion(QDataStream::Qt_5_10);

    connect(socket_, &QIODevice::readyRead, this, &ThreadWorker::readyRead, Qt::DirectConnection);
    connect(socket_, &QTcpSocket::disconnected, this, &ThreadWorker::disconnected);
    qDebug() << socketDescriptor_ << " Client connected";
}

void ThreadWorker::readyRead() {
    // get the commands from client
    in.startTransaction();
    QByteArray data;
    in >> data;
    qDebug() << data;
    if (!in.commitTransaction())
        return;
    
    auto msg = data.toStdString();

    MoveCommand cmd;
    if (!cmd.ParseFromString(msg))
        return;

    qDebug() << cmd.target().x() << " " << cmd.target().y();
    emit moveCommand(socketDescriptor_, cmd.target().x(), cmd.target().y());
}

void ThreadWorker::disconnected() {
    qDebug() << socketDescriptor_ << " Disconnected";
    emit connectionLost(socketDescriptor_);
    socket_->deleteLater();
    
    QThread::currentThread()->exit(0);
}

void ThreadWorker::sendGameState(const std::string& state) {
    // send game state to client
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    out << state.data();

    socket_->write(block);
}



