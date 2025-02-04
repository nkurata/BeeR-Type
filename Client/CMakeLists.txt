cmake_minimum_required(VERSION 3.14)
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# Local includes if needed
include_directories(
    ${CMAKE_SOURCE_DIR}/BeeR-Type/include
    ${CMAKE_SOURCE_DIR}/GameEngine/include/
    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
    ${CMAKE_SOURCE_DIR}/Client/include
)

# The Client library sources
set(CLIENT_SOURCES
    src/Client.cpp
    src/Scene.cpp
    src/LobbyScene.cpp
    src/GameScene.cpp
)

add_library(Client SHARED ${CLIENT_SOURCES})

target_link_libraries(Client
    Boost::Boost
    SFML::SFML
    ECS
)
