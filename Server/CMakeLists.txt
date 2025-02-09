cmake_minimum_required(VERSION 3.14)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories(
    ${CMAKE_SOURCE_DIR}/Server/include
    ${CMAKE_SOURCE_DIR}/Network/include
    ${CMAKE_SOURCE_DIR}/GameEngine/include
    ${CMAKE_SOURCE_DIR}/BeeR-Type/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
    ${CMAKE_SOURCE_DIR}/Server/Errors
)

# The Server library sources
set(SERVER_SOURCES
    src/Server.cpp
)

add_library(Server ${SERVER_SOURCES})

target_link_libraries(Server
    Boost::Boost
    SFML::SFML
    ECS
    Network
    GameEngine
)