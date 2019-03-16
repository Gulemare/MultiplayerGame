#pragma once
#include "../Common/unit_types.h"
#include "unit_visitor.h"
#include "map.h"

namespace game {

    class UnitInterface {

    public:
        UnitInterface(const Coords& position, int health, int player)
            : position_(position), health_(health), player_(player) {}
        virtual ~UnitInterface() = default;

        virtual UnitType getType() = 0;

        virtual void acceptVisitor(UnitVisitor& visitor) = 0;

        int getHealth() const {
            return health_;
        }

        const auto& getCoords() const {
            return position_;
        }

        int owner() const {
            return player_;
        }

        void takeDamage(int damage) {
            health_ -= damage;
        }

    protected:
        Coords position_;
        int health_;
        int player_;
    };

}