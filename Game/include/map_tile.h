#pragma once
#include "../Common/hex_utils.h"

namespace game {

    enum Terrain {
        GROUND = 0,
        MOUNTAIN
    };

    struct Tile {
        Terrain terrain = GROUND;
        bool isOccupied = false;
    };

    using Coords = Oddr<int>;
}