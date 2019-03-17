#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "constants.h"
#include "../Common/protocol.pb.h"

class UnitGraphicsItem : public QGraphicsItem {
public:
    UnitGraphicsItem(const QBrush& brush);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
        QWidget *widget) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void setUnit(const Unit& unit);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    const QBrush initialBrush_;
    const QPen initialPen_;
    QBrush brush_;
    QPen pen_;

    Unit unit_;
};