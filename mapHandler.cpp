#include "mapHandler.h"

#include <iostream>



#include <chrono>
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


MapHandler::MapHandler(vector<vector<int>> grid) 
: map(grid), rd(), gen(rd()) 
{
    getEnemyList();
    isRunning = true;
    currentLevel = 1;
    nextLvl = false;
    score = 0;
}


int MapHandler::randint(int min, int max) {
    uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


void MapHandler::getEnemyList() {
    for (int y = 0; y < map.size(); y++) {
        for (int x = 0; x < map[0].size(); x++) {
            int currentCell = map[y][x];

            if (currentCell == ENEMY) enemyList.push_back({y, x});
            
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


void MapHandler::breakWall(int direction) {
    pair<int, int> playerPos = getPlayerPos();
    switch (direction) {
        case NORTH:
            if (playerPos.first == 0) break;
            if (map[playerPos.first-1][playerPos.second] == WALL)  {
                map[playerPos.first-1][playerPos.second] = FLOOR;
            } 
            break;
        case SOUTH:
            if (playerPos.first == map.size()-1) break;
            if (map[playerPos.first+1][playerPos.second] == WALL) {
                map[playerPos.first+1][playerPos.second] = FLOOR;
            }
            break;
        case EAST:
            if (playerPos.second == map[0].size() -1 ) break;
            if (map[playerPos.first][playerPos.second+1] == WALL) {
                map[playerPos.first][playerPos.second+1] = FLOOR;
            }
            break;
        case WEST:
            if (playerPos.second == 0) break;
            if (map[playerPos.first][playerPos.second-1] == WALL) {
                map[playerPos.first][playerPos.second-1] = FLOOR;
            }
            break;
    }
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


void MapHandler::setMap(vector<vector<int>> newMap) {
    map = newMap;
    enemyList.clear();
    getEnemyList();
}


void MapHandler::update() {
    static steady_clock::time_point lastUpdate = steady_clock::now();

    steady_clock::time_point now = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - lastUpdate).count();

    if (elapsed >= 500) {  
        updateEnemies();
        lastUpdate = now;
    }

    if (getTreasureCount() == 0) nextLvl = true; currentLevel+=1;



}