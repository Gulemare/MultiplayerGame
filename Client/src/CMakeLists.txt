include_directories(
    ${GameServer_SOURCE_DIR}/Client/include
    ${GameServer_SOURCE_DIR}/Game/include
    ${GameServer_SOURCE_DIR}/Common
)

add_executable(Client
    ../../Common/protocol.pb.h ../../Common/protocol.pb.cc
    main.cpp
)

conan_target_link_libraries(Client)