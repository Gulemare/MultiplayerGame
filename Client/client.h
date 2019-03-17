#pragma once
#include <QtWidgets>
#include <QDataStream>
#include <QTcpSocket>
#include <QHash>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "game_scene.h"
#include "../Common/protocol.pb.h"

/*! Game client */
class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);

private:
    QLineEdit* hostEdit_ = nullptr;
    QLineEdit* portLineEdit_ = nullptr;
    QTextEdit* serverTextWidget_ = nullptr;
    QPushButton* connectButton_ = nullptr;
    QPushButton* endTurnButton_ = nullptr;
    QGraphicsView* view_ = nullptr;
    GameScene* scene_ = nullptr;
    

    QTcpSocket *tcpSocket_ = nullptr;
    QDataStream in;
    void initScene();

    void sendMessage(const QByteArray& msg);
    void sendCommand(const Command& cmd);

private slots:
    void connectButtonClicked();

    void sendEndTurnCommand();
    void sendMoveCommand(uint64_t unitId, const QPoint& target);
    void sendSpawnCommand(uint64_t unitId, const QPoint& targetPos, UnitType unitType);


    void getGameState();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableConnectionButton();
    void enableEndTurnButton();
};
