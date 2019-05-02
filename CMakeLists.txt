cmake_minimum_required(VERSION 3.2)

project(GameServer)
set(CMAKE_CXX_STANDARD 17)

include(build/conanbuildinfo_multi.cmake)
conan_basic_setup()

add_subdirectory(Game)
add_subdirectory(Server)
add_subdirectory(Client)





