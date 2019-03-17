#include "../Common/protocol.pb.h"
#include "client.h"
#include "constants.h"
#include <chrono>
#include <QGLWidget>

Client::Client(QWidget *parent)
    : QWidget(parent)
    , hostEdit_(new QLineEdit)
    , portLineEdit_(new QLineEdit)
    , connectButton_(new QPushButton(tr("Connect")))
    , endTurnButton_(new QPushButton(tr("End turn")))
    , serverTextWidget_(new QTextEdit(tr("Connect to server first")))
    , tcpSocket_(new QTcpSocket(this))
    , scene_(new GameScene)
    , view_(new QGraphicsView)
{
    hostEdit_->setText("127.0.0.1");

    portLineEdit_->setValidator(new QIntValidator(1, 65535, this));
    portLineEdit_->setText("1234");

    auto hostLabel = new QLabel(tr("Server name:"));
    auto portLabel = new QLabel(tr("Server port:"));

    auto quitButton = new QPushButton(tr("Quit"));
    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(connectButton_, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(hostEdit_, &QLineEdit::textChanged,
        this, &Client::enableConnectionButton);
    connect(portLineEdit_, &QLineEdit::textChanged,
        this, &Client::enableConnectionButton);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

    // Connect socket signals
    in.setDevice(tcpSocket_);
    in.setVersion(QDataStream::Qt_5_10);

    connect(tcpSocket_, &QIODevice::readyRead, this, &Client::getGameState);
    connect(tcpSocket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        this, &Client::displayError);

    connect(connectButton_, &QPushButton::clicked, this, &Client::connectButtonClicked);

    auto mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostEdit_, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit_, 1, 1);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    mainLayout->addWidget(serverTextWidget_, 4, 0, 1, 2);
    mainLayout->addWidget(endTurnButton_, 5, 0, 1, 2);
    mainLayout->addWidget(view_, 6, 0, 1, 2);

    enableConnectionButton();
    enableEndTurnButton();
    initScene();

    setMinimumSize(500, 900);
}

void Client::connectButtonClicked()
{
    connectButton_->setEnabled(false);
    tcpSocket_->abort();
    tcpSocket_->connectToHost(hostEdit_->text(),
        portLineEdit_->text().toInt());
}

void Client::sendMessage(const QByteArray& msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << msg;
    tcpSocket_->write(block);
}

void Client::sendCommand(const Command& cmd)
{
    QByteArray msg(cmd.ByteSize(), Qt::Uninitialized);
    cmd.SerializeToArray(msg.data(), msg.size());
    sendMessage(msg);
}

void Client::sendEndTurnCommand()
{
    Command cmd;
    cmd.set_allocated_end_turn(new EndTurn);
    sendCommand(cmd);
}

void Client::sendMoveCommand(uint64_t unitId, const QPoint& target)
{
    Command cmd;
    auto move = new Move();
    move->set_unit_id(unitId);
    auto pos = new Position;
    pos->set_x(target.x());
    pos->set_y(target.y());
    move->set_allocated_position(pos);
    cmd.set_allocated_move(move);
    sendCommand(cmd);
}

void Client::sendSpawnCommand(uint64_t unitId, const QPoint& targetPos, UnitType unitType)
{
    Command cmd;
    auto spawn = new Spawn();
    spawn->set_unit_id(unitId);
    auto pos = new Position;
    pos->set_x(targetPos.x());
    pos->set_y(targetPos.y());
    spawn->set_allocated_position(pos);
    spawn->set_unit_type(unitType);
    cmd.set_allocated_spawn(spawn);
    sendCommand(cmd);
}

void Client::getGameState()
{
    in.startTransaction();
    QByteArray data;
    in >> data;
    if (!in.commitTransaction())
        return;

    GameState state;
    if (!state.ParseFromArray(data.data(), data.size()))
        return;

    auto info = QString("%1, %2, ").arg(state.active_player()).arg(state.player());

    for (auto it = state.units().begin(); it != state.units().end(); ++it) {
        const auto& unit = it->second;
        info.append(QString("{%1|%2|(%3, %4)}")
            .arg(it->first)
            .arg(unit.type())
            .arg(unit.position().x())
            .arg(unit.position().y()));
    }

    endTurnButton_->setEnabled(state.player() == state.active_player());

    scene_->update(state);

    serverTextWidget_->append(info);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Client"),
            tr("The host was not found. Please check the "
                "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Client"),
            tr("The connection was refused by the peer"));
        break;
    default:
        QMessageBox::information(this, tr("Client"),
            tcpSocket_->errorString());
    }

    serverTextWidget_->append("Disconnected from server");
    scene_->clear();
    enableConnectionButton();
    enableEndTurnButton();
}

void Client::enableConnectionButton()
{
    connectButton_->setEnabled(
        !hostEdit_->text().isEmpty() &&
        !portLineEdit_->text().isEmpty());
}

void Client::enableEndTurnButton()
{
    endTurnButton_->setEnabled(tcpSocket_->isOpen());
}

void Client::initScene()
{
    view_->setViewport(new QGLWidget);
    view_->setScene(scene_);

    connect(endTurnButton_, &QPushButton::clicked, this, &Client::sendEndTurnCommand, Qt::QueuedConnection);
}
