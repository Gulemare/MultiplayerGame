#pragma once
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include "unit_interface.h"

namespace game {
    using IdToUnit = std::unordered_map<uint64_t, std::shared_ptr<UnitInterface>>;
    class UnitsHolder : private IdToUnit {
    public:
        void create(UnitType type, const Coords& pos);
        std::shared_ptr<UnitInterface> get(uint64_t id) const;

        void setUser(int player) { player_ = player; }

        auto begin() const  { return IdToUnit::begin(); };
        auto end() const { return IdToUnit::end(); }
        auto erase(IdToUnit::const_iterator it) { return IdToUnit::erase(it); }
        auto reset() {
            IdToUnit::clear();
            lastId_ = 0;
        }

    private:
        uint64_t lastId_ = 0;
        int player_ = -1;
    };

}