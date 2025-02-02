cmake_minimum_required(VERSION 3.14)
project(Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}/BeeR-Type/include
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
    src/Game.cpp
)

add_library(GameLib SHARED ${GAME_SOURCES})

target_link_libraries(GameLib
    ECSLib
    GameEngineLib
)