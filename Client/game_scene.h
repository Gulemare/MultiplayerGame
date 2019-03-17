#pragma once
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QHash>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "../Common/unit_types.h"
#include "constants.h"
#include "../Common/protocol.pb.h"
#include "hex_tile.h"

namespace std {

    template <>
    struct hash<QPoint>
    {
        std::size_t operator()(const QPoint& k) const
        {
            using std::hash;
            return hash<int>()(k.x()) + 100 * hash<int>()(k.y());
        }
    };

}

class GameScene : public QGraphicsScene
{
    Q_OBJECT

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    std::unordered_map<uint64_t, QGraphicsItem*> units_;
    std::unordered_map<QPoint, HexTile*> tiles_;
public:
    void update(const GameState& state);
    void clear();

signals:
    void clickedOnScene(QPointF pos);
};
