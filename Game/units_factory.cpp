#include "units_holder.h"
#include "worker.h"
#include <stdexcept>

using namespace game;

void UnitsHolder::create(UnitType type, const Coords& pos)
{
    std::shared_ptr<UnitInterface> newUnit;
    switch (type) {
    case WORKER: newUnit = std::make_shared<Worker>(pos, player_); break;
    default: throw std::invalid_argument("Unknown type");
    }

    insert({ lastId_++, newUnit });
}

std::shared_ptr<UnitInterface> game::UnitsHolder::get(uint64_t id) const
{
    if (count(id) == 0)
        return nullptr;
    else
        return at(id);
}
