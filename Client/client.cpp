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
    , socket_(new QWebSocket)
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
    connect(socket_, &QWebSocket::binaryMessageReceived, this, &Client::getGameState);
    connect(socket_, &QWebSocket::disconnected, this, &Client::onDisconnected);

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
    socket_->abort();
    socket_->open(QString("ws://%1:%2").arg(hostEdit_->text()).arg(portLineEdit_->text()));
}

void Client::sendMessage(const QByteArray& msg)
{
    socket_->sendBinaryMessage(msg);
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
    auto text = QString("MOVE %1 TO (%2, %3)").arg(unitId).arg(target.x()).arg(target.y());
    serverTextWidget_->append(text);
    serverTextWidget_->verticalScrollBar()->setValue(serverTextWidget_->verticalScrollBar()->maximum());

    Command cmd;
    auto move = new Move();
    move->set_unit_id(unitId);
    auto pos = new Position;
    pos->set_col(target.x());
    pos->set_row(target.y());
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
    pos->set_col(targetPos.x());
    pos->set_row(targetPos.y());
    spawn->set_allocated_position(pos);
    spawn->set_unit_type(unitType);
    cmd.set_allocated_spawn(spawn);
    sendCommand(cmd);
}

void Client::sendDirectAttackCommand(uint64_t unitId, uint64_t targetId)
{
    auto text = QString("ATTACK %1 ON %2").arg(unitId).arg(targetId);
    serverTextWidget_->append(text);
    serverTextWidget_->verticalScrollBar()->setValue(serverTextWidget_->verticalScrollBar()->maximum());

    Command cmd;
    auto attack = new DirectAttack();
    attack->set_unit_id(unitId);
    attack->set_target_id(targetId);
    cmd.set_allocated_direct_attack(attack);
    sendCommand(cmd);
}

void Client::getGameState(const QByteArray& data)
{
    GameState state;
    if (!state.ParseFromArray(data.data(), data.size()))
        return;

    auto info = QString("%1, %2, ").arg(state.active_player()).arg(state.player());

    for (auto it = state.units().begin(); it != state.units().end(); ++it) {
        const auto& unit = it->second;
        info.append(QString("{%1 : %2}")
            .arg(it->first)
            .arg(unit.health()));
    }

    endTurnButton_->setEnabled(state.player() == state.active_player());

    scene_->update(state);

    serverTextWidget_->append(info);
}

void Client::onDisconnected()
{
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
    endTurnButton_->setEnabled(true);
}

void Client::initScene()
{
    view_->setViewport(new QGLWidget);
    view_->setScene(scene_);

    connect(endTurnButton_, &QPushButton::clicked, this, &Client::sendEndTurnCommand, Qt::QueuedConnection);
    connect(scene_, &GameScene::tileSelected, [this](QPoint pos) {
        
        auto unit = scene_->getSelectedUnit();
        if (!unit)
            return;

        auto unitOnTile = scene_->getUnitOnTile(pos);
        if (unitOnTile) {
            sendDirectAttackCommand(unit->getId(), unitOnTile->getId());
        }
        else {
            sendMoveCommand(unit->getId(), pos);
        }
    });
}
