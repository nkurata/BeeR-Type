cmake_minimum_required(VERSION 3.14)
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
# target_compile_options(Client PRIVATE -g)

# Local includes if needed
include_directories(
-    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
    ${CMAKE_SOURCE_DIR}/Client/include
    ${CMAKE_SOURCE_DIR}/Client/include/Entity/
    ${CMAKE_SOURCE_DIR}/Client/include/Scene/
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
