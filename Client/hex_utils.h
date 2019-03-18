#pragma once
#include <QPoint>
#include "constants.h"

inline QPointF gridPosToSceneCoords(const QPoint& gridPos) {
    float xOffset = (gridPos.y() % 2) * HALF_WIDTH;
    float yOffset = 0.f;

    QPointF res;
    res.setX(gridPos.x() * WIDTH + xOffset);
    res.setY(gridPos.y() * HEIGHT * 0.75 + yOffset);
    return res;
}

template <typename T>
struct CubeCoords {
    T x = 0;
    T y = 0;
    T z = 0;
};

template <typename T>
struct Hex {
    T q = 0;
    T r = 0;
};

inline QPoint cubeToOddr(const CubeCoords<int>& cube) {
    auto x = cube.x + (cube.z - (cube.z % 2)) / 2;
    auto y = cube.z;
    return { x, y };
}

inline CubeCoords<int> oddrToCube(const QPoint& pos)
{
    auto x = pos.x() - (pos.y() - (pos.y() % 2)) / 2;
    auto z = pos.y();
    auto y = -x - z;
    return { x, y, z };
}

inline CubeCoords<int> cubeRound(const CubeCoords<float>& cube)
{
    auto rx = round(cube.x);
    auto ry = round(cube.y);
    auto rz = round(cube.z);

    auto x_diff = abs(rx - cube.x);
    auto y_diff = abs(ry - cube.y);
    auto z_diff = abs(rz - cube.z);

    if (x_diff > y_diff && x_diff > z_diff)
        rx = -ry - rz;
    else if (y_diff > z_diff)
        ry = -rx - rz;
    else
        rz = -rx - ry;

    return CubeCoords<int>{ (int)rx, (int)ry, (int)rz };
}

template <typename T>
CubeCoords<T> axialToCube(const Hex<T>& hex) {
    return { hex.q, -hex.r - hex.q, hex.r };
}

inline QPoint scenePosToOddr(const QPointF& point) {
    float q = (sqrt(3.f) / 3.f * point.x() - 1.f / 3.f * point.y()) / HEX_SIZE;
    float r = (2.f / 3.f * point.y()) / HEX_SIZE;

    CubeCoords<float> cubeF = axialToCube<float>({ q, r });
    return cubeToOddr(cubeRound(cubeF));
}