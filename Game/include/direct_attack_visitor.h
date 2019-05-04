#pragma once
#include <array>
#include "unit_visitor.h"
#include "worker.h"
#include "map.h"

namespace game {

    class DirectAttackVisitor : public UnitVisitor {
    public:
        DirectAttackVisitor(UnitInterface& target, const Map& map)
            : target_(target), map_(map) {}

        bool isSuccess() const {
            return success_;
        }

        void visit(Warrior& warrior) override {
            const auto pos = target_.getCoords();
            if (cubeDistance(oddrToCube(pos), oddrToCube(warrior.getCoords())) > Warrior::attackRange ||
                warrior.getActionPoints() < Warrior::attackCost)
            {
                success_ = false;
                return;
            }

            target_.setHealth(target_.getHealth() - warrior.attackPower);
            warrior.setActionPoints(warrior.getActionPoints() - Warrior::attackCost);
            success_ = true;
            return;
        }

    private:
        UnitInterface& target_;
        const Map& map_;
        bool success_ = false;
    };

}