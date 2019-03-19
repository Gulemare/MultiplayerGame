#pragma once
#include "unit_interface.h"

namespace game {
    class Worker : public UnitInterface {
    public:
        static const int maxHealth = 100;
        static const int healthRegeneration = 10;
        static const int maxActionPoints = 10;
        static const int attackPower = 20;
        static const int attackCost = 5;
        static const int attackRange = 1;

        Worker(const Coords& coords, int player)
            : UnitInterface(coords, player, maxHealth, maxActionPoints) {}

        virtual UnitType getType() override {
            return UnitType::WORKER;
        }
        virtual void accept(UnitVisitor& visitor) {
            visitor.visit(*this);
        }
    };

}