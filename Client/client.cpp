#include "../Common/protocol.pb.h"
#include "client.h"
#include "constants.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , hostEdit_(new QLineEdit)
    , portLineEdit_(new QLineEdit)
    , connectButton_(new QPushButton(tr("Connect")))
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
    mainLayout->addWidget(serverTextWidget_, 3, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 2);
    mainLayout->addWidget(view_, 5, 0, 1, 2);

    enableConnectionButton();
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

void Client::sendCommand(const QByteArray& msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << msg;
    tcpSocket_->write(block);
}

void Client::sendMoveCommand(QPointF pos)
{
    MoveCommand cmd;
    auto target = new Actor::Position;
    target->set_x(pos.x() / PIXELS_IN_METER);
    target->set_y(pos.y() / PIXELS_IN_METER);
    cmd.set_allocated_target(target);

    QByteArray msg(cmd.ByteSize(), Qt::Uninitialized);
    cmd.SerializeToArray(msg.data(), msg.size());
    sendCommand(msg);
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

    std::string res;
    std::unordered_set<uint64_t> updatedIds;
    for (int i = 0; i < state.actors_size(); ++i) {
        auto obj = state.actors(i);
        res += "{";
        res += std::to_string(obj.id()) + ", ";
        res += std::to_string(obj.type()) + ", ";
        res += std::to_string(obj.position().x()) + ", ";
        res += std::to_string(obj.position().y()) + "} ";

        updatedIds.insert(obj.id());

        if (!scene_->containsActor(obj.id())) {
            scene_->createActor(static_cast<GameType>(obj.type()), obj.id());
        }

        scene_->getActor(obj.id())->setPos({
                obj.position().x() * PIXELS_IN_METER,
                obj.position().y() * PIXELS_IN_METER });
    }

    scene_->removeNotUpdated(updatedIds);

    //serverTextWidget_->append(QString::fromStdString(res));
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

    connectButton_->setEnabled(true);
}

void Client::enableConnectionButton()
{
    connectButton_->setEnabled(
        !hostEdit_->text().isEmpty() &&
        !portLineEdit_->text().isEmpty());
}

void Client::initScene()
{
    scene_->addRect(-10 * PIXELS_IN_METER, -10 * PIXELS_IN_METER, 20 * PIXELS_IN_METER, 20 * PIXELS_IN_METER);
    view_->setScene(scene_);

    connect(scene_, &GameScene::clickedOnScene, this, &Client::sendMoveCommand, Qt::QueuedConnection);
}
