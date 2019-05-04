#pragma once

namespace game {
    class Warrior;
    class UnitVisitor {
    public:
        virtual void visit(Warrior&) = 0;
    };
}