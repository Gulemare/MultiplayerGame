#include "game_scene.h"

using UnitCreateFunc = std::shared_ptr<QGraphicsItem>(*)(Qt::GlobalColor);

std::shared_ptr<QGraphicsItem> createWorker(Qt::GlobalColor color) {
    auto worker = std::make_shared<QGraphicsEllipseItem>(
        QRect(0, 0, PIXELS_IN_TILE, PIXELS_IN_TILE));
    worker->setBrush(QBrush(color, Qt::SolidPattern));
    return worker;
}

const std::unordered_map<UnitType, UnitCreateFunc> createFuncs = {
    {UnitType::WORKER, createWorker},
};

void GameScene::update(const GameState& state)
{
    auto player = state.player();

    for (auto it = state.units().begin(); it != state.units().end(); ++it) {
        auto id = it->first;
        auto data = it->second;
        std::shared_ptr<QGraphicsItem> item = nullptr;
        if (units_.count(id) == 0) {
            // Create unit if not exist
            auto color = data.player() == player ? Qt::green : Qt::red;
            item = createFuncs.at(static_cast<UnitType>(data.type()))(color);
            
            units_[id] = item;
            addItem(item.get());
        }
        else {
            item = units_[id];
        }

        item->setPos(data.position().x() * PIXELS_IN_TILE, data.position().y() * PIXELS_IN_TILE);
        if (data.health() < 0) {
            removeItem(item.get());
            units_.erase(id);
        }
    }
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit clickedOnScene(event->scenePos());
}
