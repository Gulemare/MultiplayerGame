#pragma once

namespace game {
    enum Terrain {
        GROUND = 0,
        MOUNTAIN
    };

    struct Tile {
        Terrain terrain = GROUND;
        bool isOccupied = false;
    };

    struct Coords {
        int x;
        int y;
    };
}