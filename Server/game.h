#pragma once
#include <Box2D/Box2D.h>
#include <QString>
#include <QPoint>
#include <vector>
#include <unordered_map>

class ActorData;

class Game
{
public:
    Game();
    ~Game();

    void init();
    void tick();

    void addPlayer(QPoint pos, const QString& name);
    void removePlayer(const int id);

    void updatePlayerMovePath(const int playerId, const std::list<QPointF>& newPath);

    std::string getState();

private:
    const b2Vec2 gravity_ = { 0.f, 0.f };
    const float32 timeStep_ = 1.f / 60.f;
    const int32 velocityIterations_ = 6;
    const int32 positionIterations_ = 2;

    std::unordered_map<int, b2Body*> players_;

    b2World world_ = gravity_;

    void deleteRemoved();
    void handlePlayersMovements();
};

