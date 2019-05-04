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

        size_t getPlayerTeam(size_t player) const;
        GameState getState() const;

        bool consumeCommand(size_t player, const Command& command); // applies command and returns true if game changed

    private:
        Command lastAppliedCommand_;
        size_t playerCount_ = 0;
        size_t currentPlayerCount_ = 0;
        size_t activeTeam_ = 0;

        struct PlayerTeam {
            size_t teamId;
            bool playerIsActive;
        };

        // Player teams, in curent implementation: 0 - players team, 1 - npc team
        std::unordered_map<size_t, PlayerTeam> teams_;

        UnitsHolder units_;
        game::Map map_;

        void spawnEnemies();

        size_t activeTeam() const;

        bool applyCommand(size_t playerId, const Move& moveCommand);
        bool applyCommand(size_t playerId, const Spawn& spawnCommand);
        bool applyCommand(size_t playerId, const EndTurn& endCommand);
        bool applyCommand(size_t playerId, const DirectAttack& directAttackCommand);

        void restoreActiveTeamUnits();
        void removeDeadUnits();
    };

}

