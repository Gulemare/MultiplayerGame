#pragma once
#include <stdint.h>

enum ActorType {
    UNKNOWN = 0,
    PLAYER,
    WALL,
    REMOVED
};

struct ActorData {
    ActorData(ActorType type = UNKNOWN);
    virtual ~ActorData() {};
    uint64_t id() const;
    ActorType type() const ;
    void setRemoved();

private:
    ActorType type_;
    uint64_t id_;
    static uint64_t lastId;
};
