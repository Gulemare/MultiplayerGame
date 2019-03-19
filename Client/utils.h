#pragma once
#include <QPoint>
#include "constants.h"
#include "../Common/hex_utils.h"

inline QPointF gridPosToSceneCoords(const QPoint& gridPos) {
    float xOffset = (gridPos.y() % 2) * HALF_WIDTH;
    float yOffset = 0.f;

    QPointF res;
    res.setX(gridPos.x() * WIDTH + xOffset);
    res.setY(gridPos.y() * HEIGHT * 0.75 + yOffset);
    return res;
}

inline QPoint scenePosToOddr(const QPointF& point) {
    const auto pos = pixelToOddr({
        static_cast<float>(point.x()),
        static_cast<float>(point.y())},
        HEX_SIZE);
    return { pos.col, pos.row };
}