#pragma once
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "../Common/unit_types.h"
#include "constants.h"
#include "../Common/protocol.pb.h"

class GameScene : public QGraphicsScene
{
    Q_OBJECT

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    std::unordered_map<uint64_t, std::shared_ptr<QGraphicsItem>> units_;
public:
    void update(const GameState& state);

signals:
    void clickedOnScene(QPointF pos);
};
