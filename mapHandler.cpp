#include "mapHandler.h"

#include <iostream>



#include <chrono>
#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm> // for remove


using namespace std;
using namespace std::chrono;



#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

#define FLOOR 0
#define WALL 1
#define TREASURE 2
#define ENEMY 3
#define PLAYER 4
#define BULLET_N 5
#define BULLET_S 6
#define BULLET_E 7
#define BULLET_W 8

MapHandler::MapHandler(vector<vector<int>> grid) 
: map(grid), rd(), gen(rd()) 
{
    getEnemyAndBulletList();
    isRunning = true;
    currentLevel = 1;
    nextLvl = false;
    score = 0;
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
    switch (direction) {
        case NORTH:
            if (playerPos.first == 0) break;
            if (map[playerPos.first-1][playerPos.second] == FLOOR || map[playerPos.first-1][playerPos.second] == TREASURE) {
                if (map[playerPos.first-1][playerPos.second] == TREASURE) score += 100;
                map[playerPos.first-1][playerPos.second] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;

            } 
            break;
        case SOUTH:
            if (playerPos.first == map.size()-1) break;
            if (map[playerPos.first+1][playerPos.second] == FLOOR || map[playerPos.first+1][playerPos.second] == TREASURE) {
                if (map[playerPos.first+1][playerPos.second] == TREASURE) score += 100;
                map[playerPos.first+1][playerPos.second] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;
        case EAST:
            if (playerPos.second == map[0].size() -1 ) break;
            if (map[playerPos.first][playerPos.second+1] == FLOOR || map[playerPos.first][playerPos.second+1] == TREASURE) {
                if (map[playerPos.first][playerPos.second+1] == TREASURE) score += 100;
                map[playerPos.first][playerPos.second+1] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;
        case WEST:
            if (playerPos.second == 0) break;
            if (map[playerPos.first][playerPos.second-1] == FLOOR || map[playerPos.first][playerPos.second-1] == TREASURE) {
                if (map[playerPos.first][playerPos.second-1] == TREASURE) score += 100;
                map[playerPos.first][playerPos.second-1] = PLAYER;
                map[playerPos.first][playerPos.second] = FLOOR;
            }
            break;
    }
   
} 


int MapHandler::breakWall(int direction) {
    pair<int, int> playerPos = getPlayerPos();
    switch (direction) {
        case NORTH:
            if (playerPos.first == 0) return 0;
            if (map[playerPos.first-1][playerPos.second] == WALL)  {
                map[playerPos.first-1][playerPos.second] = FLOOR;
                return 0;
            } 

            if (map[playerPos.first-1][playerPos.second] == FLOOR || map[playerPos.first-1][playerPos.second] == ENEMY) {
                map[playerPos.first-1][playerPos.second] = BULLET_N;
                bulletList.push_back({playerPos.first-1, playerPos.second, BULLET_N});
                return 1;
            }
            
        case SOUTH:
            if (playerPos.first == map.size()-1) return 0;
            if (map[playerPos.first+1][playerPos.second] == WALL)  {
                map[playerPos.first+1][playerPos.second] = FLOOR;
                return 0;
            } 

            if (map[playerPos.first+1][playerPos.second] == FLOOR || map[playerPos.first+1][playerPos.second] == ENEMY) {
                map[playerPos.first+1][playerPos.second] = BULLET_S;
                bulletList.push_back({playerPos.first+1, playerPos.second, BULLET_S});
                return 1;
            }
        
            case EAST:
            if (playerPos.second == map[0].size() -1 ) return 0;
            if (map[playerPos.first][playerPos.second+1] == WALL)  {
                map[playerPos.first][playerPos.second+1] = FLOOR;
                return 0;
            } 

            if (map[playerPos.first][playerPos.second+1] == FLOOR || map[playerPos.first][playerPos.second+1] == ENEMY) {
                map[playerPos.first][playerPos.second+1] = BULLET_E;
                bulletList.push_back({playerPos.first, playerPos.second+1, BULLET_E});
                return 1;
            }

        case WEST:
            if (playerPos.second == 0) return 0;
            if (map[playerPos.first][playerPos.second-1] == WALL)  {
                map[playerPos.first][playerPos.second-1] = FLOOR;
                return 0;
            } 

            if (map[playerPos.first][playerPos.second-1] == FLOOR || map[playerPos.first][playerPos.second-1] == ENEMY) {
                map[playerPos.first][playerPos.second-1] = BULLET_W;
                bulletList.push_back({playerPos.first, playerPos.second-1, BULLET_W});
                return 1;
            }
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

        switch (type) {
            case BULLET_N: {
                int newY = y-1;
                int newX = x;
                
                //case FLOOR, just move normally
                if (map[newY][newX] == FLOOR ) {
                    newMap[newY][newX] = BULLET_N;
                    newMap[y][x] = FLOOR;
                    newBulletList.push_back({newY, newX, type});
                    break;
                }

                // case ENEMY, delete both enemy and bullet
                else if (map[newY][newX] == ENEMY) {
                    score += 150;
                    newMap[newY][newX] = FLOOR;
                    newMap[y][x] = FLOOR;
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(newY, newX)), enemyList.end());
                    break;
                }

                // case WALL
                else if (map[newY][newX] == WALL || map[newY][newX] == PLAYER || map[newY][newX] == TREASURE  || map[newY][newX] == BULLET_N || map[newY][newX] == BULLET_S || map[newY][newX] == BULLET_E || map[newY][newX] == BULLET_W) {
                    newMap[y][x] = FLOOR;
                    break;
                }
                break;
            }
            case BULLET_S: {
                int newY = y+1;
                int newX = x;
                
                //case FLOOR, just move normally
                if (map[newY][newX] == FLOOR) {
                    newMap[newY][newX] = BULLET_S;
                    newMap[y][x] = FLOOR;
                    newBulletList.push_back({newY, newX, type});
                    break;
                }

                // case ENEMY, delete both enemy and bullet
                else if (map[newY][newX] == ENEMY) {
                    score += 150;
                    newMap[newY][newX] = FLOOR;
                    newMap[y][x] = FLOOR;
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(newY, newX)), enemyList.end());
                    break;
                }

                // case WALL
                else if (map[newY][newX] == WALL || map[newY][newX] == PLAYER || map[newY][newX] == TREASURE  || map[newY][newX] == BULLET_N || map[newY][newX] == BULLET_S || map[newY][newX] == BULLET_E || map[newY][newX] == BULLET_W) {
                    newMap[y][x] = FLOOR;
                    break;
                }
                break;
            }
            case BULLET_E: {
                int newY = y;
                int newX = x+1;
                
                //case FLOOR, just move normally
                if (map[newY][newX] == FLOOR) {
                    newMap[newY][newX] = BULLET_E;
                    newMap[y][x] = FLOOR;
                    newBulletList.push_back({newY, newX, type});
                    break;
                }

                // case ENEMY, delete both enemy and bullet
                else if (map[newY][newX] == ENEMY) {
                    score += 150;
                    newMap[newY][newX] = FLOOR;
                    newMap[y][x] = FLOOR;
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(newY, newX)), enemyList.end());
                    break;
                }

                // case WALL
                else if (map[newY][newX] == WALL || map[newY][newX] == PLAYER|| map[newY][newX] == TREASURE  || map[newY][newX] == BULLET_N || map[newY][newX] == BULLET_S || map[newY][newX] == BULLET_E || map[newY][newX] == BULLET_W) {
                    newMap[y][x] = FLOOR;
                    break;
                }
                break;
            }
            case BULLET_W: {
                int newY = y;
                int newX = x-1;
                
                //case FLOOR, just move normally
                if (map[newY][newX] == FLOOR) {
                    newMap[newY][newX] = BULLET_W;
                    newMap[y][x] = FLOOR;
                    newBulletList.push_back({newY, newX, type});
                    break;
                }

                // case ENEMY, delete both enemy and bullet
                else if (map[newY][newX] == ENEMY) {
                    score += 150;
                    newMap[newY][newX] = FLOOR;
                    newMap[y][x] = FLOOR;
                    enemyList.erase(remove(enemyList.begin(), enemyList.end(), make_pair(newY, newX)), enemyList.end());
                    break;
                }

                // case WALL
                else if (map[newY][newX] == WALL || map[newY][newX] == PLAYER || map[newY][newX] == TREASURE  || map[newY][newX] == BULLET_N || map[newY][newX] == BULLET_S || map[newY][newX] == BULLET_E || map[newY][newX] == BULLET_W) {
                    newMap[y][x] = FLOOR;
                    break;
                }
                break;
            }
        }
    }
    map = newMap;
    bulletList = newBulletList;
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



    steady_clock::time_point now = steady_clock::now();
    auto elapsedEnemies = duration_cast<milliseconds>(now - lastUpdateEnemies).count();
    auto elapsedBullets = duration_cast<milliseconds>(now - lastUpdateBullets).count();


    if (elapsedBullets >= 100) {  
        updateBullets();
        lastUpdateBullets = now;
    }


    if (elapsedEnemies >= 500) {  
        updateEnemies();
        lastUpdateEnemies = now;
    }

    if (getTreasureCount() == 0) nextLvl = true; currentLevel+=1;



}