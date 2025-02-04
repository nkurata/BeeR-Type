cmake_minimum_required(VERSION 3.14)
project(R-Type)

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
set(RTYPE_SOURCES
    src/Client.cpp
    src/Scene.cpp
    src/LobbyScene.cpp
    src/GameScene.cpp
)

add_library(R-Type SHARED ${CLIENT_SOURCES})

# Link with R-Type if it uses game logic, Boost if needed, SFML if needed, ECSLib, etc.
target_link_libraries(R-Type
    Boost::Boost
    SFML::SFML
    ECSLib
)

set_target_properties(R-Type PROPERTIES PREFIX "" SUFFIX ".so" LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
