#pragma once
#include <QPoint>
const float PIXELS_IN_TILE = 40.f;
const auto HALF_WIDTH = PIXELS_IN_TILE / 2.f;
const auto HALF_HEIGHT = PIXELS_IN_TILE / 2.f;

inline QPointF gridPosToSceneCoords(const QPoint& gridPos) {
    float xOffset = HALF_WIDTH * (gridPos.y() % 2);
    float yOffset = HALF_HEIGHT;

    QPointF res;
    res.setX(gridPos.x() * PIXELS_IN_TILE + xOffset);
    res.setY(gridPos.y() * PIXELS_IN_TILE * 0.75 + yOffset);
    return res;
}