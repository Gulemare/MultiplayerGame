#pragma once
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include "unit_interface.h"

namespace game {

    
    class UnitsHolder {
    private:
        using IdToUnit = std::unordered_map<uint64_t, std::shared_ptr<UnitInterface>>;
        IdToUnit units_;

        uint64_t lastId_ = 1u;
        int player_ = -1;
        int team_ = -1;

    public:
        void create(UnitType type, const Coords& pos);
        std::shared_ptr<UnitInterface> get(uint64_t id) const;

        void setTeam(int team) { team_ = team; }
        void setUser(int player) { player_ = player; }

        auto begin() const { return units_.begin(); };
        auto end() const { return units_.end(); }
        auto erase(IdToUnit::const_iterator it) { return units_.erase(it); }
        auto reset() {
            units_.clear();
            lastId_ = 1u;
        }

    
    };
}
