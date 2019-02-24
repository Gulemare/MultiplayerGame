#pragma once
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "../Common/game_types.h"
#include "constants.h"

class GameScene : public QGraphicsScene
{
    Q_OBJECT

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    std::unordered_map<int, std::shared_ptr<QGraphicsItem>> actors_;

public:
    void createActor(GameType type, int id);

    std::shared_ptr<QGraphicsItem> getActor(int id);

    void removeNotUpdated(const std::unordered_set<uint64_t>& ids);

    bool containsActor(int id);

signals:
    void clickedOnScene(QPointF pos);
};
