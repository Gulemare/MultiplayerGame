#include "unit_graphics_item.h"
#include "utils.h"

UnitGraphicsItem::UnitGraphicsItem(uint64_t id, const QBrush & brush)
    : id_(id), brush_(brush), initialBrush_(brush), initialPen_(QPen())
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF UnitGraphicsItem::boundingRect() const
{
    auto penWidth = 1.;
    return QRectF(
        -HEX_SIZE * 0.8 - penWidth,
        -HEX_SIZE * 0.8 - penWidth,
        HEX_SIZE * 1.6 + penWidth * 2.,
        HEX_SIZE * 1.6 + penWidth * 2.);
}

void UnitGraphicsItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->setBrush(brush_);
    painter->setPen(pen_);
    painter->drawEllipse(-HEX_SIZE * 0.8, -HEX_SIZE * 0.8, HEX_SIZE * 1.6, HEX_SIZE * 1.6);
    painter->drawText(QPointF{ 0., 0. }, QString::number(unit_.action_points()));
}

QVariant UnitGraphicsItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (value == true)
        {
            pen_.setWidth(5);
            //brush_.setColor(Qt::yellow);
        }
        else
        {
            pen_ = initialPen_;
            brush_ = initialBrush_;
            // do stuff if not selected
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void UnitGraphicsItem::setUnit(const Unit& unit)
{
    unit_ = unit;
    setPos(gridPosToSceneCoords({
        static_cast<int>(unit_.position().col()),
        static_cast<int>(unit_.position().row()) }));
}

const Unit& UnitGraphicsItem::getData() const
{
    return unit_;
}

uint64_t UnitGraphicsItem::getId() const
{
    return id_;
}

void UnitGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    if (mouseEvent->modifiers() & Qt::ControlModifier)
    {
        mouseEvent->ignore();
    }
    else
    {
        QGraphicsItem::mousePressEvent(mouseEvent);
    }
}
