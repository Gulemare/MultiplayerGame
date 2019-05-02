include_directories(
    ${GameServer_SOURCE_DIR}/Game/include
    ${GameServer_SOURCE_DIR}/Common
)

add_library(Game STATIC
    ../../Common/protocol.pb.h ../../Common/protocol.pb.cc
    ../include/game.h game.cpp
    ../include/map.h map.cpp
    ../include/units_holder.h units_holder.cpp
    ../include/unit_interface.h
    ../include/map_tile.h
    ../include/units_holder.h
    ../include/unit_visitor.h
    ../include/movement_visitor.h
    ../include/restoration_visitor.h
    ../include/direct_attack_visitor.h
    ../include/worker.h
    )

# target_link_libraries(Game ${CONAN_LIBS})
conan_target_link_libraries(Game)
