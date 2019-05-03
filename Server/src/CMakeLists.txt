include_directories(
    ${GameServer_SOURCE_DIR}/Server/include
    ${GameServer_SOURCE_DIR}/Game/include
    ${GameServer_SOURCE_DIR}/Common
)

add_executable(Server
    ../../Common/protocol.pb.h
    main.cpp
    ../include/server.h server.cpp
)
target_link_libraries(Server Game)

conan_target_link_libraries(Server)
#target_link_libraries(Server ${CONAN_LIBS})