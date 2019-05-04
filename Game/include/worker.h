#pragma once
#include "unit_interface.h"

namespace game {
    class Warrior : public UnitInterface {
    public:
        static const int maxHealth = 100;
        static const int healthRegeneration = 15;
        static const int maxActionPoints = 10;
        static const int attackPower = 30;
        static const int attackCost = 5;
        static const int attackRange = 1;

        Warrior(const Coords& coords, int player, int team)
            : UnitInterface(coords, player, team, maxHealth, maxActionPoints) {}

        virtual UnitType getType() override {
            return UnitType::WARRIOR;
        }
        virtual void accept(UnitVisitor& visitor) {
            visitor.visit(*this);
        }
    };

}