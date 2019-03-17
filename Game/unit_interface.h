#pragma once
#include "../Common/unit_types.h"
#include "unit_visitor.h"
#include "map_tile.h"

namespace game {

    class UnitInterface {

    public:
        UnitInterface(const Coords& position, int player, int health, int actionPoints)
            : position_(position), owner_(player), health_(health), actionPoints_(actionPoints){}
        virtual ~UnitInterface() = default;

        virtual UnitType getType() = 0;
        virtual void accept(UnitVisitor& visitor) = 0;

        const auto& getCoords() const { return position_; }
        void setCoords(const Coords& newPos) { position_ = newPos;}

        int getHealth() const { return health_; }
        void setHealth(int health) { health_ = health; }

        int getActionPoints() const { return actionPoints_; }
        void setActionPoints(int actions) { actionPoints_ = actions; }

        int owner() const { return owner_; }

    protected:
        Coords position_;
        int health_;
        int owner_;
        int actionPoints_;
    };
}