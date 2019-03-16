#pragma once
#include <vector>
#include "stlastar.h"

namespace game {

    enum Terrain {
        GROUND = 0,
        MOUNTAIN
    };

    struct Coords {
        int x;
        int y;
    };

    class Map {
    public:
        Map(size_t width, size_t height);
        Terrain getTerrain(int x, int y) const;

        std::vector<Coords> getPath(const Coords& start, const Coords& goal) const;

        size_t width();
        size_t height();

    public:
        std::vector<Terrain> terrain;
        size_t width_;
        size_t height_;
        size_t coordsToIndex(size_t x, size_t y) const;
    };

}
