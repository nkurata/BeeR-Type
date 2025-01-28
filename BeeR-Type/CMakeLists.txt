cmake_minimum_required(VERSION 3.14)
project(Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}/Game/include
    ${CMAKE_SOURCE_DIR}/GameEngine/include/
    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
)

set(GAME_SOURCES
    src/Player.cpp
    src/Enemy.cpp
    src/Bullet.cpp
    src/Boss.cpp
)

add_library(GameLib ${GAME_SOURCES})

target_link_libraries(GameLib
    ECSLib
)