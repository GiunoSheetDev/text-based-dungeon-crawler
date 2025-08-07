#include "caveGenerator.h"
#include <iostream>


using namespace std;

#define FLOOR 0
#define WALL 1
#define TREASURE 2
#define ENEMY 3
#define PLAYER 4


CaveGenerator::CaveGenerator(int width, int heigth) 
:   rd(), gen(rd()), width(width), heigth(heigth),
    cellmap(heigth, vector<int>(width))
{
    fillProb = 0.40;
    iterations = 3;
    cellmap = vector<vector<int>> (heigth, vector<int>(width));
    lvl = 1;
    enemyLimit = randint(7, 12) + 5*lvl;
}

int CaveGenerator::randint(int min, int max) {
    uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


vector<vector<int>> CaveGenerator::initMap() {
    for (int y = 1; y < (int)cellmap.size() - 1; y++) {
        for (int x = 1; x < (int)cellmap[y].size() - 1; x++) {
            cellmap[y][x] = randint(FLOOR, WALL);
        }
    }



    //borders with walls
    for (int y = 0; y < cellmap.size(); y++) {
        cellmap[y][0] = WALL;
        cellmap[y][cellmap[0].size()-1] = WALL;
    }

    for (int x = 0; x < cellmap[0].size(); x++) {
        cellmap[0][x] = WALL;
        cellmap[cellmap.size()-1][x] = WALL;
    }

    return cellmap;

}

int CaveGenerator::getNeighbors(int y, int x) {
    int out = 0;

    for (int row = y-1; row < y+2; row++) {
        for (int col = x-1; col < x+2; col++) {

            //borders
            if (row < 0 || col < 0 || row >= (int)cellmap.size() || col >= (int)cellmap[row].size())
                continue;



            if (row == y && col == x) continue;
            int currentCell = cellmap[row][col];
            if (currentCell == FLOOR) out += 1;
        }
    }

    return out;

}

void CaveGenerator::generationStep() {
    vector<vector<int>> newCellmap = cellmap;

    for (int y = 0; y < cellmap.size(); y++) {
        for (int x = 0; x < cellmap[y].size(); x++) {
            int currentCell = cellmap[y][x];
            int neighbors = getNeighbors(y, x);


            if (currentCell == WALL) {
                if (neighbors < 5) newCellmap[y][x] = FLOOR;
                else newCellmap[y][x] = WALL;
            }
            else {
                if (neighbors > 2) newCellmap[y][x] = WALL;
                else newCellmap[y][x] = FLOOR;
            }
        }
    }

    cellmap = newCellmap;

}

void CaveGenerator::placeTreasures() {
    int placed = 0;
    int maxAttempts = 1000;  // Prevent infinite loops
    int attempts = 0;

    while (placed < enemyLimit && attempts < maxAttempts) {
        int y = randint(1, cellmap.size() - 2);
        int x = randint(1, cellmap[0].size() - 2);

        int currentCell = cellmap[y][x];

        if (currentCell == FLOOR) {
            int neighbors = getNeighbors(y, x);
            if (neighbors < 5) {  
                cellmap[y][x] = TREASURE;
                placed++;
            }
        }

        attempts++;
    }
}


void CaveGenerator::placeEnemies() {
    int placed = 0;
    int maxAttempts = 1000;  // Prevent infinite loops
    int attempts = 0;
    int neighborLimit = 7;

    while (placed < enemyLimit && attempts < maxAttempts) {
        if (attempts > 200) neighborLimit = 6;
        if (attempts > 400) neighborLimit = 5;
        int y = randint(1, cellmap.size() - 2);
        int x = randint(1, cellmap[0].size() - 2);

        int currentCell = cellmap[y][x];

        if (currentCell == FLOOR) {
            int neighbors = getNeighbors(y, x);
            if (neighbors < 5) {  
                cellmap[y][x] = ENEMY;
                placed++;
            }
        }

        attempts++;
    }

}

void CaveGenerator::placePlayer() {
    for (int y = 0; y < cellmap.size(); y++) {
        for (int x = 0; x < cellmap[0].size(); x++) {
            int currentCell = cellmap[y][x];

            if (currentCell == FLOOR) {
                cellmap[y][x] = PLAYER;
                return;
            } 
        }
    }
}





vector<vector<int>> CaveGenerator::generateMap(int lvl) {
    enemyLimit = randint(7, 12) + randint(0, 2)*lvl;
    cellmap = initMap();
    for (int i = 0; i <= iterations; i++) {
        generationStep();
    }
    placeTreasures();
    placeEnemies();
    placePlayer();

    return cellmap;

}



void CaveGenerator::print() {
    for (const vector<int>& row : cellmap) {
        for (int val : row) {
            switch (val)
            {
            case FLOOR:
                cout << " . ";
                break;
            
            case WALL:
                cout << "###";
                break;
            
            case TREASURE:
                cout << " $ ";
                break;
            
            case ENEMY:
                cout << " ! ";
                break;
            }

            
        }
        cout << endl;
    }
    cout << endl;
}









