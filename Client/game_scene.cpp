#include "game_scene.h"
#include "constants.h"

using UnitCreateFunc = UnitGraphicsItem*(*)(uint64_t, Qt::GlobalColor);

UnitGraphicsItem* createWorker(uint64_t id, Qt::GlobalColor color) {
    auto worker = new UnitGraphicsItem(id, QBrush(color, Qt::SolidPattern));
    return worker;
}

const std::unordered_map<UnitType, UnitCreateFunc> createFuncs = {
    {UnitType::WORKER, createWorker},
};

GameScene::GameScene()
{
    connect(this, &QGraphicsScene::selectionChanged, this, &GameScene::onSelectionChanged);
}

void GameScene::update(const GameState& state)
{
    auto player = state.player();

    for (auto it = state.units().begin(); it != state.units().end(); ++it) {
        auto id = it->first;
        auto data = it->second;
        UnitGraphicsItem* item = nullptr;
        if (units_.count(id) == 0) {
            // Create unit if not exist
            auto color = data.player() == player ? Qt::green : Qt::red;
            item = createFuncs.at(static_cast<UnitType>(data.type()))(id, color);
            
            units_[id] = item;
            addItem(item);
        }
        else {
            item = units_[id];
        }

        // Update/Remove unit
        item->setUnit(data);
        if (data.health() <= 0) {
            removeItem(item);
            units_.erase(id);
        }
    }

    for (int i = 0; i < state.tiles_size(); ++i) {
        const auto& data = state.tiles(i);

        QPoint pos(data.pos().col(), data.pos().row());

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

    QGraphicsScene::update(sceneRect());
}

void GameScene::clear()
{
    QGraphicsScene::clear();
    units_.clear();
    tiles_.clear();
}

void GameScene::onSelectionChanged()
{
    auto items = selectedItems();

    if (items.empty()) {
        if (lastSelectedItem) {
            lastSelectedItem->setSelected(true);
            return;
        }
    }
    
    lastSelectedItem = items.front();
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    auto mousePos = event->scenePos();

    if (event->buttons() & Qt::RightButton)
    {
        emit tileSelected(scenePosToOddr(mousePos));
    }
}

UnitGraphicsItem* GameScene::getSelectedUnit() const
{
    if (!lastSelectedItem)
        return nullptr;
    
    auto item = dynamic_cast<UnitGraphicsItem*>(lastSelectedItem);
    return item;
}

UnitGraphicsItem* GameScene::getUnitOnTile(const QPoint& pos) const
{
    UnitGraphicsItem* res = nullptr;
    for (const auto& p : units_) {
        const auto& coords = p.second->getData().position();
        if (coords.col() == pos.x() && coords.row() == pos.y())
            return p.second;
    }
    return res;
}

