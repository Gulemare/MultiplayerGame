#pragma once
#include <QGraphicsPolygonItem>
#include "constants.h"

class HexTile : public QGraphicsPolygonItem {
public:
    HexTile(const QPoint& gridPos) {
        gridPos_ = gridPos;
        QPolygonF vertices;
        center_ = gridPosToSceneCoords(gridPos);
        vertices <<
            QPointF{ 0., -HALF_HEIGHT } <<    // min y vertice and clockwise
            QPointF{ HALF_WIDTH, -HALF_HEIGHT / 2. } <<
            QPointF{ HALF_WIDTH, HALF_HEIGHT / 2. } <<
            QPointF{ 0., HALF_HEIGHT } <<
            QPointF{ -HALF_WIDTH, HALF_HEIGHT / 2. } <<
            QPointF{ -HALF_WIDTH, -HALF_HEIGHT / 2. };

        setPolygon(vertices);
        setPos(center_);
    }

    QPointF center() { return center_; }

private:
    QPoint gridPos_;
    QPointF center_;
};