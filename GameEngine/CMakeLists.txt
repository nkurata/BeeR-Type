cmake_minimum_required(VERSION 3.14)
project(R-Type_Library)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# (Optional) Include directories if needed
include_directories(
    ${CMAKE_SOURCE_DIR}/Server/include
    ${CMAKE_SOURCE_DIR}/BeeR-Type/include
    ${CMAKE_SOURCE_DIR}/GameEngine/include/
    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
)

# The R-Type "core" library sources
set(RTYPE_SOURCES
    src/AGame.cpp
    src/PlayerAction.cpp
)

add_library(R-Type ${RTYPE_SOURCES})

# Link against Boost::Boost, SFML::SFML, ECSLib if needed
target_link_libraries(R-Type
    Boost::Boost
    SFML::SFML
    ECSLib
)
