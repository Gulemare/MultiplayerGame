#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "../Common/protocol.pb.h"

class UnitGraphicsItem : public QGraphicsItem {
public:
    UnitGraphicsItem(uint64_t id, const QBrush& brush);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
        QWidget *widget) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void setUnit(const Unit& unit);

    const Unit& getData() const;
    uint64_t getId() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    const QBrush initialBrush_;
    const QPen initialPen_;
    QBrush brush_;
    QPen pen_;

    uint64_t id_;
    Unit unit_;
};