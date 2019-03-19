#pragma once
#include <cmath>

template <typename T>
struct Oddr {
    T col = 0;
    T row = 0;
    bool operator==(const Oddr<T>& rhs) const {
        return row == rhs.row && col == rhs.col;
    }
};

template <typename T>
struct Hex {
    T q = 0;
    T r = 0;
};

template <typename T>
struct Cube {
    T x = 0;
    T y = 0;
    T z = 0;
};

template <typename T>
inline Oddr<T> cubeToOddr(const Cube<T>& cube) {
    const auto col = cube.x + (cube.z - (cube.z % 2)) / 2;
    const auto row = cube.z;
    return { col, row };
}

template <typename T>
inline Cube<T> oddrToCube(const Oddr<T>& pos)
{
    const auto x = pos.col - (pos.row - (pos.row % 2)) / 2;
    const auto z = pos.row;
    const auto y = -x - z;
    return { x, y, z };
}

inline Cube<int> cubeRound(const Cube<float>& cube)
{
    auto rx = round(cube.x);
    auto ry = round(cube.y);
    auto rz = round(cube.z);

    const auto x_diff = abs(rx - cube.x);
    const auto y_diff = abs(ry - cube.y);
    const auto z_diff = abs(rz - cube.z);

    if (x_diff > y_diff && x_diff > z_diff)
        rx = -ry - rz;
    else if (y_diff > z_diff)
        ry = -rx - rz;
    else
        rz = -rx - ry;

    return Cube<int>{ (int)rx, (int)ry, (int)rz };
}

template <typename T>
Cube<T> axialToCube(const Hex<T>& hex) {
    return { hex.q, -hex.r - hex.q, hex.r };
}

inline Oddr<int> pixelToOddr(const Oddr<float>& point, const float hexSize) {
    const float q = (sqrt(3.f) / 3.f * point.col - 1.f / 3.f * point.row) / hexSize;
    const float r = (2.f / 3.f * point.row) / hexSize;

    const Cube<float> cubeF = axialToCube<float>({ q, r });
    return cubeToOddr(cubeRound(cubeF));
}

inline int cubeDistance(const Cube<int>& a, const Cube<int>& b) {
    return (abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z)) / 2;
}
