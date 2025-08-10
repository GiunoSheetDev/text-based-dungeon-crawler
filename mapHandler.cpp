#include "mapHandler.h"

#include <iostream>



#include <chrono>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm> 
#include <map>


using namespace std;
using namespace std::chrono;


#include "tiletypes.h"



MapHandler::MapHandler(vector<vector<int>> grid)
    : grid(grid), currentLevel(1), gen(rd()) {
    isRunning = true;
    nextLvl = false;
    score = 0;
    isTimeStopped = false;
    isOmniDirectionActive = false;
    isXDirectionActive = false;
    isCrossDirectionActive = false;
}


int MapHandler::randint(int min, int max) {
    uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


void MapHandler::getEnemyAndBulletList() {
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid[0].size(); x++) {
            int currentCell = grid[y][x];

            if (currentCell == ENEMY) enemyList.push_back({y, x});
            if (currentCell == BULLET_N || currentCell == BULLET_S || currentCell == BULLET_E || currentCell == BULLET_W) bulletList.push_back({y, x, currentCell});
        }

    }
}

pair<int, int> MapHandler::getPlayerPos() {
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid[0].size(); x++) {
            if (grid[y][x] == PLAYER) return {y, x};
        }
    }
    isRunning = false;
    return {-1, -1};
}
void MapHandler::movePlayer(int direction) {
    pair<int, int> playerPos = getPlayerPos();
    int targetY = playerPos.first;
    int targetX = playerPos.second;
    int targetCell;

    switch (direction) {
        case NORTH:
            targetY = playerPos.first - 1;
            if (targetY < 0) break;
            targetCell = grid[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);
                
                grid[targetY][targetX] = PLAYER;
                grid[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case SOUTH:
            targetY = playerPos.first + 1;
            if (targetY >= (int)grid.size()) break;
            targetCell = grid[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);

                grid[targetY][targetX] = PLAYER;
                grid[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case EAST:
            targetX = playerPos.second + 1;
            if (targetX >= (int)grid[0].size()) break;
            targetCell = grid[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);

                grid[targetY][targetX] = PLAYER;
                grid[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case WEST:
            targetX = playerPos.second - 1;
            if (targetX < 0) break;
            targetCell = grid[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell); //TODO set instead of  targetCell >= POWERUP_STOPTIME also an upper bound if i add something with value grater thatn powerup that isnt a powerup

                grid[targetY][targetX] = PLAYER;
                grid[playerPos.first][playerPos.second] = FLOOR;
            }
            break;
    }
}

int MapHandler::breakWall(int direction) {
    pair<int, int> playerPos = getPlayerPos();

    // Direction vectors: {dy, dx} for N, S, E, W
    const static std::map<int, pair<int,int>> dirOffsets = {
        {NORTH, {-1, 0}}, {SOUTH, {1, 0}}, {EAST, {0, 1}}, {WEST, {0, -1}}
    };

    // Bullet types matching directions
    const static map<int, int> bulletTypes = {
        {NORTH, BULLET_N}, {SOUTH, BULLET_S}, {EAST, BULLET_E}, {WEST, BULLET_W}
    };

    if (dirOffsets.find(direction) == dirOffsets.end()) return 0;

    int targetY = playerPos.first + dirOffsets.at(direction).first;
    int targetX = playerPos.second + dirOffsets.at(direction).second;

    // Boundary check
    if (targetY < 0 || targetY >= (int)grid.size() || targetX < 0 || targetX >= (int)grid[0].size()) 
        return 0;

    int targetCell = grid[targetY][targetX];

    if (targetCell == WALL) {
        grid[targetY][targetX] = FLOOR;
        return 0;
    }

    if (targetCell != FLOOR && targetCell != ENEMY) return 0;

    // Remove enemy if present
    if (targetCell == ENEMY) {
        enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(targetY, targetX)), enemyList.end());
    }

    // Lambda to add bullet and update grid
    auto addBullet = [&](int y, int x, int bulletType) {
        if (y >= 0 && y < (int)grid.size() && x >= 0 && x < (int)grid[0].size()) {
            grid[y][x] = bulletType;
            bulletList.push_back({y, x, bulletType});
        }
    };

    // Fire main bullet
    addBullet(targetY, targetX, bulletTypes.at(direction));

    // Omni-directional bullets
    if (isOmniDirectionActive) {
        static const vector<tuple<int,int,int>> omniOffsets = {
            {1, 0, BULLET_S}, {-1, 0, BULLET_N}, {0, 1, BULLET_E}, {0, -1, BULLET_W},
            {-1, 1, BULLET_NE}, {-1, -1, BULLET_NW}, {1, 1, BULLET_SE}, {1, -1, BULLET_SW}
        };
        for (auto& [dy, dx, bType] : omniOffsets)
            addBullet(playerPos.first + dy, playerPos.second + dx, bType);
        return 1;
    }

    // Cross-directional bullets (N, S, E, W except main direction)
    if (isCrossDirectionActive) {
        static const vector<tuple<int,int,int>> crossOffsets = {
            {1, 0, BULLET_S}, {-1, 0, BULLET_N}, {0, 1, BULLET_E}, {0, -1, BULLET_W}
        };
        for (auto& [dy, dx, bType] : crossOffsets) {
            // Skip main direction bullet (already fired)
            if (dy == dirOffsets.at(direction).first && dx == dirOffsets.at(direction).second)
                continue;
            addBullet(playerPos.first + dy, playerPos.second + dx, bType);
        }
    }

    // X-directional bullets (diagonal)
    if (isXDirectionActive) {
        static const vector<tuple<int,int,int>> xOffsets = {
            {-1, 1, BULLET_NE}, {-1, -1, BULLET_NW}, {1, 1, BULLET_SE}, {1, -1, BULLET_SW}
        };
        for (auto& [dy, dx, bType] : xOffsets)
            addBullet(playerPos.first + dy, playerPos.second + dx, bType);
    }

    return 1;
}




int MapHandler::getTreasureCount() {
    int out = 0; 
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid[0].size(); x++) {
            if (grid[y][x] == TREASURE) out += 1;
        }
    }
    return out;
}



void MapHandler::updateEnemies() {
    vector<vector<int>> newMap = grid;          // copy of the current grid
    vector<pair<int,int>> newEnemyList;        // new list for updated enemy positions

    for (auto& coords : enemyList) {
        int y = coords.first;
        int x = coords.second;

        vector<pair<int,int>> possibleMoves;

        // Check neighbors for FLOOR cells only (no enemies or walls)
        if (y > 0 && (newMap[y - 1][x] == FLOOR || newMap[y - 1][x] == PLAYER)) {
            possibleMoves.push_back({y - 1, x});
        }
        if (y < (int)newMap.size() - 1 && (newMap[y + 1][x] == FLOOR || newMap[y + 1][x] == PLAYER)) {
            possibleMoves.push_back({y + 1, x});
        }
        if (x > 0 && (newMap[y][x - 1] == FLOOR || newMap[y][x - 1] == PLAYER)) {
            possibleMoves.push_back({y, x - 1});
        }
        if (x < (int)newMap[0].size() - 1 && (newMap[y][x + 1] == FLOOR || newMap[y][x + 1] == PLAYER)) {
            possibleMoves.push_back({y, x + 1});
        }

        if (!possibleMoves.empty()) {
            // Choose random move
            int index = randint(0, (int)possibleMoves.size() - 1);
            pair<int,int> newCoords = possibleMoves[index];

            if (newMap[newCoords.first][newCoords.second] == PLAYER) isRunning = false;

            // Move enemy in newMap
            newMap[y][x] = FLOOR;                     // clear old position
            newMap[newCoords.first][newCoords.second] = ENEMY;

            // Save new enemy position
            newEnemyList.push_back(newCoords);
        } else {
            // No valid moves: enemy stays in place
            newEnemyList.push_back(coords);
        }
    }

    // Update grid and enemy list
    grid = newMap;
    enemyList = newEnemyList;
}
void MapHandler::updateBullets() {
    vector<vector<int>> newMap = grid;
    vector<vector<int>> newBulletList;

    for (auto& coords : bulletList) {
        int y = coords[0];
        int x = coords[1];
        int type = coords[2];

        int newY = y;
        int newX = x;

        // Calculate new position based on bullet type
        switch (type) {
            case BULLET_N: newY = y - 1; break;
            case BULLET_S: newY = y + 1; break;
            case BULLET_E: newX = x + 1; break;
            case BULLET_W: newX = x - 1; break;
            case BULLET_NE: newX = x+1; newY = y-1; break;
            case BULLET_SE: newY = y+1; newX = x+1; break;
            case BULLET_SW: newY = y+1; newX = x-1; break;
            case BULLET_NW: newY = y-1; newX = x-1; break;
        }

        // Check boundaries: if out of bounds, remove bullet (do nothing further)
        if (newY < 0 || newY >= (int)grid.size() || newX < 0 || newX >= (int)grid[0].size()) {
            newMap[y][x] = FLOOR;  // Remove bullet from old position
            continue;              // Skip to next bullet
        }

        
        int cell = grid[newY][newX];
        if (cell == FLOOR) {
            newMap[newY][newX] = type;
            newMap[y][x] = FLOOR;
            newBulletList.push_back({newY, newX, type});
        }
        else if (cell == ENEMY) {
            score += 150;
            newMap[newY][newX] = FLOOR;
            newMap[y][x] = FLOOR;
            enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(newY, newX)), enemyList.end());
        }
        else {
            // Hits wall, player, treasure, or another bullet: bullet disappears
            newMap[y][x] = FLOOR;
        }
    }

    grid = newMap;
    bulletList = newBulletList;
}


void MapHandler::executePowerUp(int powerup) {
    switch (powerup)  {
        case POWERUP_STOPTIME:
            isTimeStopped = true;
            break;
        case POWERUP_OMNIDIRECTIONALBULLETS:
            isOmniDirectionActive = true;
            break;
        case POWERUP_XDIRECTIONALBULLETS:
            isXDirectionActive = true;
            break;
        case POWERUP_CROSSDIRECTIONALBULLETS:
            isCrossDirectionActive = true;
            break;


    }
}



void MapHandler::setMap(vector<vector<int>> newMap) {
    grid = newMap;
    enemyList.clear();
    bulletList.clear();
    getEnemyAndBulletList();
}


void MapHandler::update() {
    static steady_clock::time_point lastUpdateEnemies = steady_clock::now() - milliseconds(500);
    static steady_clock::time_point lastUpdateBullets = steady_clock::now() - - milliseconds(100);;

    // Timer for time stop powerup
    static steady_clock::time_point timeStopStart;
    static bool timeStopTimerStarted = false;

    // Timer for omni-directional bullets powerup
    static steady_clock::time_point omniStart;
    static bool omniTimerStarted = false;

    static steady_clock::time_point xStart;
    static bool xTimerStarted = false;
    
    static steady_clock::time_point crossStart;
    static bool crossTimerStarted = false;

    steady_clock::time_point now = steady_clock::now();
    auto elapsedEnemies = duration_cast<milliseconds>(now - lastUpdateEnemies).count();
    auto elapsedBullets = duration_cast<milliseconds>(now - lastUpdateBullets).count();

    // Handle time stop timer
    if (isTimeStopped) {
        if (!timeStopTimerStarted) {
            timeStopStart = now;
            timeStopTimerStarted = true;
        } else if (duration_cast<seconds>(now - timeStopStart).count() >= 10) {
            isTimeStopped = false;
            timeStopTimerStarted = false;
        }
    }

    // Handle omni-directional bullets timer
    if (isOmniDirectionActive) {
        if (!omniTimerStarted) {
            omniStart = now;
            omniTimerStarted = true;
        } else if (duration_cast<seconds>(now - omniStart).count() >= 10) {
            isOmniDirectionActive = false;
            omniTimerStarted = false;
        }
    }

    // Handle X-directional bullets timer
    if (isXDirectionActive) {
        if (!xTimerStarted) {
            xStart = now;
            xTimerStarted = true;
        } else if (duration_cast<seconds>(now - xStart).count() >= 10) {
            isXDirectionActive = false;
            xTimerStarted = false;
        }
    }


    if (isCrossDirectionActive) {
        if (!crossTimerStarted) {
            crossStart = now;
            crossTimerStarted = true;
        } else if (duration_cast<seconds>(now - crossStart).count() >= 10) {
            isCrossDirectionActive = false;
            crossTimerStarted = false;
        }
    }

    if (elapsedBullets >= 100) {  
        updateBullets();
        lastUpdateBullets = now;
    }

    if (!isTimeStopped) {
        if (elapsedEnemies >= 500) {  
            updateEnemies();
            lastUpdateEnemies = now;
        }
    }

    if (getTreasureCount() == 0) {
        nextLvl = true; 
        currentLevel += 1;
    }
}
