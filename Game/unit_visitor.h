#pragma once
namespace game {

    class Worker;
    class UnitVisitor {
    public:
        virtual void visit(Worker&) = 0;
    };

}