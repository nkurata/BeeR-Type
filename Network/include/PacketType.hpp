#pragma once

#include <cstdint>

#include "Packet.hpp"

namespace Network {
    enum class PacketType {
        NONE = 0,
        REQCONNECT = 1,
        DISCONNECTED = 2,
        GAME_START = 3,
        PLAYER_DEAD = 4,
        PLAYER_JOIN = 5,
        PLAYER_SHOOT = 6,

        GAME_END = 15,
        PLAYER_RIGHT = 16,
        PLAYER_LEFT = 17,
        PLAYER_UP = 18,
        PLAYER_DOWN = 19,

        CREATE_ENEMY = 22,
        CREATE_BOSS = 23,
        CREATE_PLAYER = 24,
        CREATE_BULLET = 25,

        CHANGE = 29,
        GAME_STARTED = 30,
        GAME_NOT_STARTED = 31,
        UI_UPDATE = 34,
        DESTROY_PLAYER = 35,
        DESTROY_ENEMY = 36,
        DESTROY_BOSS = 37,
        DESTROY_BULLET = 38,

        HEARTBEAT = 99,
    };
}
