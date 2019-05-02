#pragma once
#include "unit_visitor.h"
#include "worker.h"

namespace game {
    /*! Restore action points to units */
    class RestorationVisitor : public UnitVisitor {
    public:
        void visit(Worker& worker) override {
            worker.setActionPoints(Worker::maxActionPoints);

            auto newHealth = worker.getHealth() + Worker::healthRegeneration;
            worker.setHealth(std::min(Worker::maxHealth, newHealth));
        }
    };
}
