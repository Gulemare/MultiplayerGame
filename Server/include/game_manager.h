#pragma once
/*
#include <memory>
#include <boost/signals2.hpp>
#include "protocol.pb.h"
#include "game.h"

class GameManager : public std::enable_shared_from_this<GameManager> {
    game::Game game_;
    int i = 0;

    using UpdateHandler = boost::signals2::signal<void(int)>;
    UpdateHandler updateSignal_; //!< Signal to update observers with new game state

public:
    void subscribe(UpdateHandler::slot_type handler) { updateSignal_.connect(handler); }

    void consumeCommand() {
        updateSignal_(++i);
    }
};
*/