#include "actor.h"
uint64_t ActorData::lastId = 0;

ActorData::ActorData(ActorType type) : type_(type) {
    id_ = lastId++;
}

uint64_t ActorData::id() const
{
    return id_;
}

ActorType ActorData::type() const
{
    return type_;
}

void ActorData::setRemoved()
{
    type_ = REMOVED;
}
