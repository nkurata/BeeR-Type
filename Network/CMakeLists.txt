cmake_minimum_required(VERSION 3.14)
project(Network)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories(
    ${CMAKE_SOURCE_DIR}/Server/include
    ${CMAKE_SOURCE_DIR}/BeeR-Type/include
    ${CMAKE_SOURCE_DIR}/GameEngine/include/
    ${CMAKE_SOURCE_DIR}/ECS
    ${CMAKE_SOURCE_DIR}/ECS/systems
    ${CMAKE_SOURCE_DIR}/ECS/components
)

# Network sources
set(NETWORK_SOURCES
    src/PacketHandler.cpp
    )

set(NETWORK_HEADERS
    include/PacketHandler.hpp
    include/Data.hpp
    include/Packet.hpp
    include/PacketType.hpp
    include/ThreadSafeQueue.hpp
)

# Create Network library
add_library(Network ${NETWORK_SOURCES} ${NETWORK_HEADERS})

# Include directories
target_include_directories(Network PUBLIC include)

# Link dependencies
target_link_libraries(Network
    Boost::Boost
    SFML::SFML
    R-Type
)
