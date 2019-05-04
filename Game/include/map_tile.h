#pragma once
#include "../Common/hex_utils.h"

namespace game {

    enum Terrain {
        GROUND = 0,
        MOUNTAIN
    };

    struct Tile {
        Terrain terrain = GROUND;
        size_t occupiedBy = 0; // 0 - not occupied
    };

    using Coords = Oddr<int>;
}