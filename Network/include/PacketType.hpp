/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** PacketType
*/

#pragma once

#include <cstdint>

#include "Packet.hpp"

namespace Network {
    enum class PacketType {
        NONE = 0,
        REQCONNECT = 1,
        CONNECTED = 2,
        DISCONNECTED = 3,
        GAME_START = 4,
        GAME_END = 5,
        KILL = 6,
        CHANGE = 7,
        
        PLAYER_CREATE = 10,
        ENEMY_CREATE = 11,
        BULLET_CREATE = 12,
        BOSS_CREATE = 13,

        PLAYER_UP = 20,
        PLAYER_DOWN = 21,
        PLAYER_LEFT = 22,
        PLAYER_RIGHT = 23,
        PLAYER_STOP_U = 24,
        PLAYER_STOP_D = 25,
        PLAYER_STOP_L = 26,
        PLAYER_STOP_R = 27,
        PLAYER_SHOOT = 28,
        PLAYER_BLAST = 29,
        
        HEARTBEAT = 99,
    };
}
