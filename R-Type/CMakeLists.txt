cmake_minimum_required(VERSION 3.14)
project(R-Type)

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

set(RTYPE_SOURCES
    src/Players.cpp
    src/Enemy.cpp
    src/Bullet.cpp
    src/Boss.cpp
    src/Game.cpp
)

add_library(R-Type ${RTYPE_SOURCES})

target_link_libraries(R-Type
    Boost::Boost
    SFML::SFML
    ECS
    GameEngine
)

# set_target_properties(R-Type PROPERTIES PREFIX "" SUFFIX ".so" LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
