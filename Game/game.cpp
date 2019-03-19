#include "game.h"
#include "movement_visitor.h"
#include "direct_attack_visitor.h"
#include "../Common/protocol.pb.h"

using namespace game;

Game::Game() : map_(10, 10)
{
    restart(2);
}

Game::~Game()
{
}

void Game::restart(const size_t playerCount)
{
    playerCount_ = playerCount;
    activePlayer_ = 0;
    currentPlayerCount_ = 0;
    units_.reset();
}

bool Game::consumeCommand(size_t player, const Command& command)
{
    if (player != activePlayer_)
        return false;

    removeDeadUnits();

    units_.setUser(activePlayer_);

    bool gameChanged = false;

    if (command.has_end_turn()) {
        gameChanged = applyCommand(command.end_turn());
    }
    else if (command.has_spawn()) {
        gameChanged = applyCommand(command.spawn());
    }
    else if (command.has_move()) {
        gameChanged = applyCommand(command.move());
    }
    else if (command.has_direct_attack()) {
        gameChanged = applyCommand(command.direct_attack());
    }

    if (!gameChanged)
        return false;
    
    map_.updateOccupied(units_);
    return gameChanged;
}

size_t Game::activePlayer() const
{
    return activePlayer_;
}

void Game::addPlayer()
{
    if (started())
        return;

    // Create starting units for player
    units_.setUser(currentPlayerCount_);
    units_.create(UnitType::WORKER, { static_cast<int>(currentPlayerCount_), 0 });

    currentPlayerCount_++;
}

bool Game::started()
{
    return currentPlayerCount_ == playerCount_;
}

GameState Game::getState() const
{
    GameState state;

    state.set_active_player(activePlayer());
    state.set_player(-1);   // Invalid player, should be reset by server for each client
    

    for (auto it = units_.begin(); it != units_.end(); ++it) {
        const auto id = it->first;
        auto data = it->second;
        Unit unit;
        unit.set_type(data->getType());
        unit.set_health(data->getHealth());
        unit.set_action_points(data->getActionPoints());
        unit.set_player(data->owner());
        
        auto pos = new Position();
        pos->set_col(data->getCoords().col);
        pos->set_row(data->getCoords().row);
        unit.set_allocated_position(pos);
        (*state.mutable_units())[id] = unit;
    }

    for (int row = 0; row < map_.height(); ++row) {
        for (int col = 0; col < map_.width(); ++col) {
            auto tile = state.add_tiles();
            auto pos = new Position;
            pos->set_col(col);
            pos->set_row(row);
            tile->set_allocated_pos(pos);
            tile->set_terrain(map_.getTile({ col, row}).terrain);
            tile->set_occupied(map_.getTile({ col, row }).isOccupied);
        }
    }


    return state;
}

bool Game::applyCommand(const Move& moveCommand)
{
    auto unit = units_.get(moveCommand.unit_id());
    if (!unit)
        return false;

    if (unit->owner() != activePlayer_)
        return false;

    const auto& pos = moveCommand.position();
    const Coords goal{ static_cast<int>(pos.col()), static_cast<int>(pos.row()) };

    MovementVisitor move(goal, map_);
    unit->accept(move);
    return move.isSuccess();
}

bool Game::applyCommand(const Spawn& spawnCommand)
{
    return false;
}

bool Game::applyCommand(const EndTurn& endTurnCommand)
{
    activePlayer_++;
    if (activePlayer_ >= playerCount_)
        activePlayer_ = 0;

    restoreActivePlayerUnits();
    return true;
}

bool game::Game::applyCommand(const DirectAttack& directAttackCommand)
{
    auto attacker = units_.get(directAttackCommand.unit_id());
    if (!attacker)
        return false;

    if (attacker->owner() != activePlayer_)
        return false;

    auto target = units_.get(directAttackCommand.target_id());
    if (!target)
        return false;

    if (attacker->owner() == target->owner())
        return false;

    DirectAttackVisitor attack(*target, map_);
    attacker->accept(attack);
    return attack.isSuccess();
}

void Game::restoreActivePlayerUnits()
{
    RestorationVisitor restoration;
    for (auto it = units_.begin(); it != units_.end(); ++it)
    {
        auto& unit = *(it->second);
        if (unit.owner() == activePlayer_)
            unit.accept(restoration);
    }
}

void Game::removeDeadUnits()
{
    for (auto it = units_.begin(); it != units_.end();)
    {
        if (it->second->getHealth() <= 0)
            it = units_.erase(it);
        else
            ++it;
    }
}
