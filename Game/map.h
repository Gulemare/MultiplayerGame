#pragma once
#include <vector>
#include "stlastar.h"
#include "units_holder.h"
#include "map_tile.h"

namespace game {
    class Map {
    public:
        Map(size_t width, size_t height);
        Tile getTile(int x, int y) const;
        void updateOccupied(const UnitsHolder& units);

        std::vector<Coords> getPath(const Coords& start, const Coords& goal) const;

        size_t width() const;
        size_t height() const;

    public:
        std::vector<Tile> tiles_;
        size_t width_;
        size_t height_;
        size_t coordsToIndex(size_t x, size_t y) const;
    };

}
