#include "game_scene.h"

using ActorsCreateFunc = std::shared_ptr<QGraphicsItem>(*)();

std::shared_ptr<QGraphicsItem> createPlayer() {
    return std::make_shared<QGraphicsEllipseItem>(
        QRect(-1 * PIXELS_IN_METER, -1 * PIXELS_IN_METER, 2 * PIXELS_IN_METER, 2 * PIXELS_IN_METER));
}

std::shared_ptr<QGraphicsItem> createAnotherPlayer() {
    return std::make_shared<QGraphicsEllipseItem>(-1 * PIXELS_IN_METER, -1 * PIXELS_IN_METER,
        2 * PIXELS_IN_METER, 2 * PIXELS_IN_METER);
}

std::shared_ptr<QGraphicsItem> createUnknown() {
    std::shared_ptr<QGraphicsItem> res = 
        std::make_shared<QGraphicsEllipseItem>(-3, -3, 6, 6);
    res->setVisible(false);
    return res;
}

const std::unordered_map<GameType, ActorsCreateFunc> createFuncs = {
    {GameType::PLAYER, createPlayer},
    {GameType::UNKNOWN, createUnknown}
};

void GameScene::createActor(GameType type, int id)
{
    auto item = createFuncs.at(type)();
    actors_[id] = item;
    addItem(item.get());
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit clickedOnScene(event->scenePos());
}

std::shared_ptr<QGraphicsItem> GameScene::getActor(int id)
{
    return actors_.at(id);
}

void GameScene::removeNotUpdated(const std::unordered_set<uint64_t>& ids)
{
    for (auto it = actors_.begin(); it != actors_.end();) {
        if (ids.count(it->first) == 0) {
            removeItem(it->second.get());
            it = actors_.erase(it);
        }
        else
            ++it;
    }
}

bool GameScene::containsActor(int id)
{
    return actors_.count(id) != 0;
}
