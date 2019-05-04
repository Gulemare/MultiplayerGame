#include "units_holder.h"
#include "worker.h"
#include <stdexcept>

using namespace game;

void UnitsHolder::create(UnitType type, const Coords& pos)
{
    std::shared_ptr<UnitInterface> newUnit;
    switch (type) {
    case WARRIOR: newUnit = std::make_shared<Warrior>(pos, player_, team_); break;
    default: throw std::invalid_argument("Unknown type");
    }

    units_.insert({ lastId_++, newUnit });
}

std::shared_ptr<UnitInterface> game::UnitsHolder::get(uint64_t id) const
{
    if (units_.count(id) == 0)
        return nullptr;
    else
        return units_.at(id);
}
