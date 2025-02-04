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
        DISCONNECTED = 2,
        GAME_START = 3,
        PLAYER_DEAD = 4,
        PLAYER_JOIN = 5,
        PLAYER_SHOOT = 6,
        PLAYER_HIT = 7,
        PLAYER_SCORE = 8,
        ENEMY_SPAWNED = 9,
        ENEMY_DEAD = 10,
        ENEMY_MOVED = 11,
        ENEMY_SHOOT = 12,
        ENEMY_LIFE_UPDATE = 13,
        MAP_UPDATE = 14,
        GAME_END = 15,      //  //no pos
        OPEN_MENU = 20,     //no pos
        MOUSE_CLICK = 21,   //pos
        CREATE_ENEMY = 22,
        CREATE_BOSS = 23,
        CREATE_PLAYER = 24,
        CREATE_BULLET = 25,
        BACKGROUND = 26,
        CREATE_POWERUP = 27,
        DELETE = 28,
        CHANGE = 29,
        GAME_STARTED = 30,
        GAME_NOT_STARTED = 31,
        UI_UPDATE = 32,
        HEARTBEAT = 33,
        GAME_START_2 = 34,
        PLAYER_LEFT = 35,
        PLAYER_RIGHT = 36,
        PLAYER_UP = 37,
        PLAYER_DOWN = 38,
        PLAYER_STOP_LEFT = 39,
        PLAYER_STOP_RIGHT = 40,
        PLAYER_STOP_UP = 41,
        PLAYER_STOP_DOWN = 42,
    };
}
