#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include "map.h"
#include "unit_interface.h"
#include "units_holder.h"
#include "protocol.pb.h"

namespace game {

    class Game
    {
    public:
        explicit Game(size_t playerCount = 2);
        ~Game();

        void restart(const size_t playerCount = 2); // Initialize game
        int addPlayer();                            // Add new player to game simulation, returns player id
        bool started();                             // Game is started when all players are added

        GameState getState() const;

        bool consumeCommand(size_t player, const Command& command); // applies command and returns true if game changed

    private:
        Command lastAppliedCommand_;
        size_t playerCount_ = 0;
        size_t currentPlayerCount_ = 0;
        size_t activePlayer_ = 0;

        UnitsHolder units_;
        game::Map map_;

        size_t activePlayer() const;

        bool applyCommand(const Move& moveCommand);
        bool applyCommand(const Spawn& spawnCommand);
        bool applyCommand(const EndTurn& endCommand);
        bool applyCommand(const DirectAttack& directAttackCommand);

        void restoreActivePlayerUnits();
        void removeDeadUnits();
    };

}

