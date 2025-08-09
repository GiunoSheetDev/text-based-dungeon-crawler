#include "mapHandler.h"

#include <iostream>



#include <chrono>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm> 


using namespace std;
using namespace std::chrono;


#include "tiletypes.h"



MapHandler::MapHandler(vector<vector<int>> grid) 
: map(grid), rd(), gen(rd()) 
{
    getEnemyAndBulletList();
    isRunning = true;
    currentLevel = 1;
    nextLvl = false;
    score = 0;
    isTimeStopped = false;
    isOmniDirectionActive = false;
}


int MapHandler::randint(int min, int max) {
    uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


void MapHandler::getEnemyAndBulletList() {
    for (int y = 0; y < map.size(); y++) {
        for (int x = 0; x < map[0].size(); x++) {
            int currentCell = map[y][x];

            if (currentCell == ENEMY) enemyList.push_back({y, x});
            if (currentCell == BULLET_N || currentCell == BULLET_S || currentCell == BULLET_E || currentCell == BULLET_W) bulletList.push_back({y, x, currentCell});
        }

    }
}

pair<int, int> MapHandler::getPlayerPos() {
    for (int y = 0; y < map.size(); y++) {
        for (int x = 0; x < map[0].size(); x++) {
            if (map[y][x] == PLAYER) return {y, x};
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
            targetCell = map[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);
                
                map[targetY][targetX] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case SOUTH:
            targetY = playerPos.first + 1;
            if (targetY >= (int)map.size()) break;
            targetCell = map[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);

                map[targetY][targetX] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case EAST:
            targetX = playerPos.second + 1;
            if (targetX >= (int)map[0].size()) break;
            targetCell = map[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell);

                map[targetY][targetX] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;

        case WEST:
            targetX = playerPos.second - 1;
            if (targetX < 0) break;
            targetCell = map[targetY][targetX];
            if (targetCell == FLOOR || targetCell == TREASURE || targetCell >= POWERUP_STOPTIME) {
                if (targetCell == TREASURE) score += 100;
                if (targetCell >= POWERUP_STOPTIME) executePowerUp(targetCell); //TODO set instead of  targetCell >= POWERUP_STOPTIME also an upper bound if i add something with value grater thatn powerup that isnt a powerup

                map[targetY][targetX] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;
    }
}


int MapHandler::breakWall(int direction) {
    pair<int, int> playerPos = getPlayerPos();

    int targetY, targetX, targetCell;

    switch (direction) {
        case NORTH:
            targetY = playerPos.first - 1;
            targetX = playerPos.second;
            if (targetY < 0) return 0;
            targetCell = map[targetY][targetX];

            if (targetCell == WALL) {
                map[targetY][targetX] = FLOOR;
                return 0;
            }

            if (targetCell == FLOOR || targetCell == ENEMY) {
                if (targetCell == ENEMY) {
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(targetY, targetX)), enemyList.end());
                }
                
                map[targetY][targetX] = BULLET_N;
                bulletList.push_back({targetY, targetX, BULLET_N});
                if (isOmniDirectionActive) {
                    map[playerPos.first+1][playerPos.second] = BULLET_S;
                    bulletList.push_back({playerPos.first+1, playerPos.second, BULLET_S});
                    map[playerPos.first][playerPos.second + 1] = BULLET_E;
                    bulletList.push_back({playerPos.first, playerPos.second + 1, BULLET_E});
                    map[playerPos.first][playerPos.second - 1] = BULLET_W;
                    bulletList.push_back({playerPos.first, playerPos.second - 1, BULLET_W});
                }

                return 1;
            }
            break;

        case SOUTH:
            targetY = playerPos.first + 1;
            targetX = playerPos.second;
            if (targetY >= (int)map.size()) return 0;
            targetCell = map[targetY][targetX];

            if (targetCell == WALL) {
                map[targetY][targetX] = FLOOR;
                return 0;
            }

            if (targetCell == FLOOR || targetCell == ENEMY) {
                if (targetCell == ENEMY) {
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(targetY, targetX)), enemyList.end());
                }
                map[targetY][targetX] = BULLET_S;
                bulletList.push_back({targetY, targetX, BULLET_S});
                if (isOmniDirectionActive) {
                    map[playerPos.first - 1][playerPos.second] = BULLET_N;
                    bulletList.push_back({playerPos.first - 1, playerPos.second, BULLET_N});
                    map[playerPos.first][playerPos.second + 1] = BULLET_E;
                    bulletList.push_back({playerPos.first, playerPos.second + 1, BULLET_E});
                    map[playerPos.first][playerPos.second - 1] = BULLET_W;
                    bulletList.push_back({playerPos.first, playerPos.second - 1, BULLET_W});
                }
                return 1;
            }
            break;

        case EAST:
            targetY = playerPos.first;
            targetX = playerPos.second + 1;
            if (targetX >= (int)map[0].size()) return 0;
            targetCell = map[targetY][targetX];

            if (targetCell == WALL) {
                map[targetY][targetX] = FLOOR;
                return 0;
            }

            if (targetCell == FLOOR || targetCell == ENEMY) {
                if (targetCell == ENEMY) {
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(targetY, targetX)), enemyList.end());
                }
                map[targetY][targetX] = BULLET_E;
                bulletList.push_back({targetY, targetX, BULLET_E});
                if (isOmniDirectionActive) {
                    map[playerPos.first - 1][playerPos.second] = BULLET_N;
                    bulletList.push_back({playerPos.first - 1, playerPos.second, BULLET_N});
                    map[playerPos.first + 1][playerPos.second] = BULLET_S;
                    bulletList.push_back({playerPos.first + 1, playerPos.second, BULLET_S});
                    map[playerPos.first][playerPos.second - 1] = BULLET_W;
                    bulletList.push_back({playerPos.first, playerPos.second - 1, BULLET_W});
                }
                return 1;
            }
            break;

        case WEST:
            targetY = playerPos.first;
            targetX = playerPos.second - 1;
            if (targetX < 0) return 0;
            targetCell = map[targetY][targetX];

            if (targetCell == WALL) {
                map[targetY][targetX] = FLOOR;
                return 0;
            }

            if (targetCell == FLOOR || targetCell == ENEMY) {
                if (targetCell == ENEMY) {
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(targetY, targetX)), enemyList.end());
                }
                map[targetY][targetX] = BULLET_W;
                bulletList.push_back({targetY, targetX, BULLET_W});
                 if (isOmniDirectionActive) {
                    map[playerPos.first - 1][playerPos.second] = BULLET_N;
                    bulletList.push_back({playerPos.first - 1, playerPos.second, BULLET_N});
                    map[playerPos.first + 1][playerPos.second] = BULLET_S;
                    bulletList.push_back({playerPos.first + 1, playerPos.second, BULLET_S});
                    map[playerPos.first][playerPos.second + 1] = BULLET_E;
                    bulletList.push_back({playerPos.first, playerPos.second + 1, BULLET_E});
                }
                return 1;
            }
            break;
    }

    return 0;
}



int MapHandler::getTreasureCount() {
    int out = 0; 
    for (int y = 0; y < map.size(); y++) {
        for (int x = 0; x < map[0].size(); x++) {
            if (map[y][x] == TREASURE) out += 1;
        }
    }
    return out;
}



void MapHandler::updateEnemies() {
    vector<vector<int>> newMap = map;          // copy of the current map
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

    // Update map and enemy list
    map = newMap;
    enemyList = newEnemyList;
}
void MapHandler::updateBullets() {
    vector<vector<int>> newMap = map;
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
        }

        // Check boundaries: if out of bounds, remove bullet (do nothing further)
        if (newY < 0 || newY >= (int)map.size() || newX < 0 || newX >= (int)map[0].size()) {
            newMap[y][x] = FLOOR;  // Remove bullet from old position
            continue;              // Skip to next bullet
        }

        
        int cell = map[newY][newX];
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

    map = newMap;
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

    }
}



void MapHandler::setMap(vector<vector<int>> newMap) {
    map = newMap;
    enemyList.clear();
    bulletList.clear();
    getEnemyAndBulletList();
}


void MapHandler::update() {
    static steady_clock::time_point lastUpdateEnemies = steady_clock::now();
    static steady_clock::time_point lastUpdateBullets = steady_clock::now();

    // Timer for time stop powerup
    static steady_clock::time_point timeStopStart;
    static bool timeStopTimerStarted = false;

    // Timer for omni-directional bullets powerup
    static steady_clock::time_point omniStart;
    static bool omniTimerStarted = false;

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
