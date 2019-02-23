#pragma once
#include <QtWidgets>
#include <QDataStream>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <memory>
#include <unordered_map>
#include <algorithm>

/*! Scene for game visualising */
class GameScene : public QGraphicsScene {
    Q_OBJECT
protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        emit clickedOnScene(event->scenePos());
    }

signals:
    void clickedOnScene(QPointF pos);
};

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
    QGraphicsView* view_ = nullptr;
    GameScene* scene_ = nullptr;
    std::unordered_map<int, std::shared_ptr<QGraphicsItem>> items_;

    QTcpSocket *tcpSocket_ = nullptr;
    QDataStream in;
    void initScene();

private slots:
    void connectButtonClicked();
    void sendCommand(const std::string& state);
    void sendMoveCommand(QPointF pos);
    void getGameState();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableConnectionButton();
};
