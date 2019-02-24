#pragma once
#include <stdint.h>

#include "../Common/game_types.h"

struct ActorData {
    ActorData(GameType type = UNKNOWN);
    virtual ~ActorData() {};
    uint64_t id() const;
    GameType type() const ;
    void setRemoved();

private:
    GameType type_;
    uint64_t id_;
    static uint64_t lastId;
};
