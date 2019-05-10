#include "game.h"
#include "movement_visitor.h"
#include "direct_attack_visitor.h"
#include "restoration_visitor.h"
#include "../Common/protocol.pb.h"

using namespace game;

const auto playersTeam = 0;
const auto npcTeam = 1;

Game::Game(const size_t playerCount) : map_(10, 8)
{
    restart(playerCount);
}

Game::~Game()
{
}

void Game::restart(const size_t playerCount)
{
    playerCount_ = playerCount;
    activeTeam_ = 0;
    currentPlayerCount_ = 0;
    units_.reset();
    teams_.clear();
    lastAppliedCommand_.Clear();
}

bool Game::checkWinConditions() const
{
    for (const auto&[id, unit] : units_) {
        if (unit->team() == npcTeam)
            return false;
    }
    return true;
}

bool Game::consumeCommand(size_t player, const Command& command)
{
    if (teams_.count(player) == 0)
        return false;

    if (teams_[player].teamId != activeTeam_)
        return false;

    removeDeadUnits();

    bool gameChanged = false;

    if (command.has_end_turn()) {
        gameChanged = applyCommand(player, command.end_turn());
    }
    else if (command.has_spawn()) {
        gameChanged = applyCommand(player, command.spawn());
    }
    else if (command.has_move()) {
        gameChanged = applyCommand(player, command.move());
    }
    else if (command.has_direct_attack()) {
        gameChanged = applyCommand(player, command.direct_attack());
    }

    if (!gameChanged)
        return false;

    lastAppliedCommand_ = command;
    
    map_.updateOccupied(units_);
    return gameChanged;
}

void game::Game::spawnEnemies()
{
    teams_.insert({ playerCount_, {npcTeam, true} });
    units_.setTeam(npcTeam);
    units_.setUser(playerCount_);
    units_.create(UnitType::WARRIOR, { 4, 3 });
    units_.create(UnitType::WARRIOR, { 4, 4 });
    units_.create(UnitType::WARRIOR, { 4, 5 });
    units_.create(UnitType::WARRIOR, { 5, 3 });
    units_.create(UnitType::WARRIOR, { 5, 4 });
    units_.create(UnitType::WARRIOR, { 5, 5 });
    map_.updateOccupied(units_);
}

size_t Game::activeTeam() const
{
    return activeTeam_;
}

int Game::addPlayer()
{
    if (started())
        return -1;

    // Create starting units for player
    const auto playerId = currentPlayerCount_;
    teams_.insert({ playerId, {playersTeam, true} });
    units_.setTeam(playersTeam);
    units_.setUser(playerId);
    units_.create(UnitType::WARRIOR, { 0, static_cast<int>(playerId) * 6 });
    units_.create(UnitType::WARRIOR, { 0, static_cast<int>(playerId) * 6 + 1 });
    map_.updateOccupied(units_);

    currentPlayerCount_++;

    if (started())
        spawnEnemies();

    return playerId;
}

bool Game::started()
{
    return currentPlayerCount_ == playerCount_;
}

size_t game::Game::getPlayerTeam(size_t player) const
{
    return teams_.at(player).teamId;
}

size_t game::Game::getActiveTeam() const
{
    return activeTeam_;
}

GameState Game::getState() const
{
    GameState state;
    state.set_active_team(activeTeam_);

    for (const auto&[id, playerTeam] : teams_) {
        if (playerTeam.teamId == activeTeam_ && playerTeam.playerIsActive)
            state.add_active_players(id);
    }

    state.set_team(-1);     // Invalid team, should be reset by server for each client
    state.set_player(-1);   // Invalid player, should be reset by server for each client

    for (const auto&[id, data]: units_) {
        Unit unit;
        unit.set_type(data->getType());
        unit.set_health(data->getHealth());
        unit.set_action_points(data->getActionPoints());
        unit.set_player(data->owner());
        unit.set_team(data->team());
        
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
            tile->set_occupied(map_.getTile({ col, row }).occupiedBy);
        }
    }

    state.set_allocated_last_command(new Command(lastAppliedCommand_));

    return state;
}

bool Game::applyCommand(size_t playerId, const Move& moveCommand)
{
    auto unit = units_.get(moveCommand.unit_id());
    if (!unit)
        return false;

    if (unit->team() != activeTeam_)
        return false;

    if (unit->owner() != playerId)
        return false;

    if (!teams_.at(playerId).playerIsActive)
        return false;

    const auto& pos = moveCommand.position();
    const Coords goal{ static_cast<int>(pos.col()), static_cast<int>(pos.row()) };

    MovementVisitor move(goal, map_);
    unit->accept(move);
    return move.isSuccess();
}

bool Game::applyCommand(size_t playerId, const Spawn& spawnCommand)
{
    return false;
}

bool Game::applyCommand(size_t playerId, const EndTurn& endTurnCommand)
{
    bool teamStillActive = false;
    bool isPlayerTurnEnd = false;
    std::set<int> teamIds;

    for (auto&[id, teamInfo] : teams_) {
        auto&[teamId, isActive] = teamInfo;
        teamIds.insert(teamId);
        if (teamId != activeTeam_)
            continue;
        if (id == playerId) {
            isActive = false;
            isPlayerTurnEnd = true;
        }

        teamStillActive |= isActive;
    }

    if (teamStillActive)
        return isPlayerTurnEnd;

    auto it = teamIds.find(activeTeam_);
    if (it == std::prev(teamIds.end()))
        activeTeam_ = *teamIds.begin();
    else
        activeTeam_ = *std::next(it);

    for (auto&[id, teamInfo] : teams_) {
        auto&[teamId, isActive] = teamInfo;
        if (teamId == activeTeam_)
            isActive = true;
    }
    
    units_.setTeam(activeTeam_);

    restoreActiveTeamUnits();
    return true;
}

bool game::Game::applyCommand(size_t playerId, const DirectAttack& directAttackCommand)
{
    auto attacker = units_.get(directAttackCommand.unit_id());
    if (!attacker)
        return false;

    if (attacker->team() != activeTeam_)
        return false;

    if (attacker->owner() != playerId)
        return false;

    if (!teams_.at(playerId).playerIsActive)
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

void Game::restoreActiveTeamUnits()
{
    RestorationVisitor restoration;
    for (auto it = units_.begin(); it != units_.end(); ++it)
    {
        auto& unit = *(it->second);
        if (unit.team() == activeTeam_)
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
