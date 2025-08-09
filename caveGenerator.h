#ifndef CAVE_GENERATOR_H
#define CAVE_GENERATOR_H


#include <vector>
#include <random>

using namespace std;


class CaveGenerator {


public:
    int width;
    int heigth;
    int lvl;
    int iterations;

    explicit CaveGenerator(int width, int heigth);

    vector<vector<int>> cellmap;
    vector<vector<int>> generateMap(int lvl);
    void print();

private:
    random_device rd;
    mt19937 gen;
    double fillProb;
    int enemyLimit;
    int powerUpLimit;
    
    
    int randint(int min, int max);
    int steps;
    
    
    int getNeighbors(int y, int x);
    vector<vector<int>> initMap();
    void generationStep();
    void placeTreasures();
    void placePowerUps();
    void placeEnemies();
    void placePlayer();


};



#endif