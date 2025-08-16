from enum import IntEnum

class Tile(IntEnum):
    FLOOR = 0
    WALL = 1
    TREASURE = 2
    ENEMY = 3
    PLAYER = 4
    BULLET = 5
    POWERUP= 6