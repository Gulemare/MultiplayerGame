#pragma once
#include "unit_visitor.h"
#include "worker.h"

namespace game {
    /*! Restore action points to units */
    class RestorationVisitor : public UnitVisitor {
    public:
        void visit(Warrior& warrior) override {
            warrior.setActionPoints(Warrior::maxActionPoints);

            auto newHealth = warrior.getHealth() + Warrior::healthRegeneration;
            warrior.setHealth(std::min(Warrior::maxHealth, newHealth));
        }
    };
}
