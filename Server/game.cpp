#include <QDebug>
#include "game.h"
#include "../Common/protocol.pb.h"
#include "actor.h"

struct PlayerData : public ActorData {

    PlayerData(const QString& name, int playerId) : playerId_(playerId), ActorData(PLAYER) {}
    virtual ~PlayerData() { qDebug() << "Player data deleted"; }
    int playerId() { return playerId_; }

    std::list<QPointF> movePath_;
private:
    int playerId_;
};

Game::Game()
{
    init();
}

Game::~Game()
{
}

void Game::init()
{
    /*
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(0.f, 0.f);
    b2Body* body = world_.CreateBody(&bodyDef);
    body->SetUserData(new ActorData);

    b2ChainShape boundBox;
    b2Vec2 corners[] = {
        {-10.f, -10.f},
        {-10.f, 10.f},
        {10.f, 10.f},
        {10.f, -10.f}};
    
    boundBox.CreateLoop(corners, std::size(corners));

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boundBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);

    */
    
}

void Game::tick()
{
    world_.Step(timeStep_, velocityIterations_, positionIterations_);
    deleteRemoved();
    handlePlayersMovements();
}

void Game::addPlayer(QPoint pos, const QString& name)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(pos.x(), pos.y());
    b2Body* body = world_.CreateBody(&bodyDef);
    auto data = new PlayerData{ "Player", name.toInt() };
    body->SetUserData(data);

    players_[data->playerId()] = body;

    b2CircleShape circleShape;
    circleShape.m_radius = 1.f;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);
}

void Game::removePlayer(const int id)
{
    auto body = players_.at(id);
    auto data = static_cast<ActorData*>(body->GetUserData());
    auto player = dynamic_cast<PlayerData*>(data);
    player->setRemoved();
}

void Game::updatePlayerMovePath(const int playerId, const std::list<QPointF>& newPath)
{
    auto body = players_.at(playerId);
    auto data = static_cast<ActorData*>(body->GetUserData());
    auto player = dynamic_cast<PlayerData*>(data);
    player->movePath_ = newPath;
}

std::string Game::getState()
{
    GameState state;
    
    for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
    {
        const auto pos = b->GetPosition();
        const auto data = static_cast<ActorData*>(b->GetUserData());

        auto obj = state.add_actors();
        auto coords = new Actor::Position();
        coords->set_x(pos.x);
        coords->set_y(pos.y);
        obj->set_allocated_position(coords);
        obj->set_type(data->type());
        obj->set_id(data->id());
    }

    std::string msg;
    state.SerializeToString(&msg);
    return msg;
}

void Game::deleteRemoved()
{
    std::vector<b2Body*> bodiesToRemove;
    for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
    {
        auto data = static_cast<ActorData*>(b->GetUserData());
        if (data->type() == REMOVED) {

            auto player = dynamic_cast<PlayerData*>(data);
            if (player)
                players_.erase(player->playerId());

            delete data;
            bodiesToRemove.push_back(b);
        }

    }

    for (auto body : bodiesToRemove) {
        world_.DestroyBody(body);
    }
}

void Game::handlePlayersMovements()
{
    for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
    {
        auto data = static_cast<ActorData*>(b->GetUserData());
        auto player = dynamic_cast<PlayerData*>(data);
        if (player) {
            auto& path = player->movePath_;
            if (path.empty()) {
                b->SetLinearVelocity({ 0.f, 0.f });
                continue;
            }
            const auto nextPoint = path.front();
            const auto dx = static_cast<float>(nextPoint.x()) - b->GetPosition().x;
            const auto dy = static_cast<float>(nextPoint.y()) - b->GetPosition().y;
            const auto distance = sqrt(dx * dx + dy * dy);
            if (distance < 1.f) {
                path.pop_front();
                continue;
            }
            b->SetLinearVelocity({ 50.f * dx / distance, 50.f * dy / distance });
        }
    }
}
