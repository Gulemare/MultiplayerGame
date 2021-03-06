#pragma once
#include <array>
#include "unit_visitor.h"
#include "worker.h"
#include "map.h"
#include "map_tile.h"

namespace game {

    const Coords oddrDirections[2][6] = {
        {{+1, 0},{0, -1},{-1, -1},{-1, 0},{-1, +1},{0, +1}},
        {{+1, 0},{+1, -1},{0, -1},{-1, 0},{0, +1},{+1, +1}},
    };

    Coords oddrOffsetNeighbor(const Coords& pos, size_t direction) {
        auto parity = pos.row % 2;
        auto dir = oddrDirections[parity][direction];
        return Coords{ pos.col + dir.col, pos.row + dir.row };
    }

    std::array<Coords, 6> getNeighbours(const Coords& pos) {
        std::array<Coords, 6> res;
        for (int i = 0; i < res.size(); ++i)
            res[i] = oddrOffsetNeighbor(pos, i);
        return res;
    }

    /*! Trying to move unit to goal position */
    class MovementVisitor : public UnitVisitor {
    public:
        MovementVisitor(const Coords& goal, const Map& map)
            : goal_(goal), map_(map) {}

        bool isSuccess() const {
            return success_;
        }

        void visit(Warrior& warrior) override {
            if (warrior.getActionPoints() == 0 ||
                !map_.isValidCoord(goal_) ||
                map_.getTile(goal_).occupiedBy > 0)
            {
                success_ = false;
                return;
            }

            const auto& coords = warrior.getCoords();
            auto neighbors = getNeighbours(coords);
            for (const auto& n : neighbors) {
                if (goal_ == n) {
                    success_ = true;
                    warrior.setCoords(goal_);
                    warrior.setActionPoints(warrior.getActionPoints() - 1);
                    return;
                }
            }

            success_ = false;
        }

    private:
        bool success_ = false;
        const Coords goal_;
        const Map& map_;
    };
}
