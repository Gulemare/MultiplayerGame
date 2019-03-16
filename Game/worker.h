#pragma once
#include "unit_interface.h"

namespace game {
    class Worker : public UnitInterface {
    public:
        Worker(const Coords& coords, int player) : UnitInterface(coords, 100, player) {}

        virtual UnitType getType() override {
            return UnitType::WORKER;
        }
        virtual void acceptVisitor(UnitVisitor& visitor) {
            visitor.visit(*this);
        }
    };

}