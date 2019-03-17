#include "game_scene.h"

using UnitCreateFunc = QGraphicsItem*(*)(Qt::GlobalColor);

QGraphicsItem* createWorker(Qt::GlobalColor color) {
    auto worker = new QGraphicsEllipseItem(
        QRect(-HALF_WIDTH * 0.8, -HALF_HEIGHT * 0.8, PIXELS_IN_TILE * 0.8, PIXELS_IN_TILE* 0.8));
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
        QGraphicsItem* item = nullptr;
        if (units_.count(id) == 0) {
            // Create unit if not exist
            auto color = data.player() == player ? Qt::green : Qt::red;
            item = createFuncs.at(static_cast<UnitType>(data.type()))(color);
            
            units_[id] = item;
            addItem(item);
        }
        else {
            item = units_[id];
        }

        // Update/Remove unit
        item->setPos(gridPosToSceneCoords({
            static_cast<int>(data.position().x()),
            static_cast<int>(data.position().y()) }));
        if (data.health() < 0) {
            removeItem(item);
            units_.erase(id);
        }
    }

    for (int i = 0; i < state.tiles_size(); ++i) {
        const auto& data = state.tiles(i);

        QPoint pos(data.pos().x(), data.pos().y());

        HexTile* item = nullptr;
        if (tiles_.count(pos) == 0) {
            // Create tile if not exist
            item = new HexTile(pos);
            item->setZValue(-100.);
            tiles_[pos] = item;
            addItem(item);
        }
        else {
            item = tiles_[pos];
        }
        // Update tile here
    }
}

void GameScene::clear()
{
    QGraphicsScene::clear();
    units_.clear();
    tiles_.clear();
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit clickedOnScene(event->scenePos());
}
