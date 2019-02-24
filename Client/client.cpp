#include "../Common/protocol.pb.h"
#include "client.h"

const float PIXELS_IN_METER = 20.f;

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
    mainLayout->addWidget(view_, 0, 2, 5, 1);

    enableConnectionButton();
    initScene();

    setMinimumSize(1500, 800);
}

void Client::connectButtonClicked()
{
    connectButton_->setEnabled(false);
    tcpSocket_->abort();
    tcpSocket_->connectToHost(hostEdit_->text(),
        portLineEdit_->text().toInt());
}

void Client::sendCommand(const std::string& msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << msg.data();
    tcpSocket_->write(block);
}

void Client::sendMoveCommand(QPointF pos)
{
    MoveCommand cmd;
    auto target = new Actor::Position;
    target->set_x(pos.x() / PIXELS_IN_METER);
    target->set_y(pos.y() / PIXELS_IN_METER);
    qDebug() << target->x() << " " << target->y();
    cmd.set_allocated_target(target);
    sendCommand(cmd.SerializeAsString());
}

void Client::getGameState()
{
    in.startTransaction();
    QByteArray data;
    in >> data;
    if (!in.commitTransaction())
        return;

    auto message = data.toStdString();
    GameState state;
    if (!state.ParseFromString(message))
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

        if (items_.count(obj.id()) == 0) {
            auto item = std::make_shared<QGraphicsEllipseItem>(
                QRect(-1 * PIXELS_IN_METER, -1 * PIXELS_IN_METER, 2 * PIXELS_IN_METER, 2 * PIXELS_IN_METER));
            item->setVisible(true);
            scene_->addItem(item.get());
            items_[obj.id()] = item;
        }
        else {
            items_[obj.id()]->setPos({
                obj.position().x() * PIXELS_IN_METER,
                obj.position().y() * PIXELS_IN_METER });
        }
    }

    for (auto it = items_.begin(); it != items_.end();) {

        if (updatedIds.count(it->first) == 0)
            it = items_.erase(it);
        else
            ++it;
    }

    serverTextWidget_->append(QString::fromStdString(res));
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
