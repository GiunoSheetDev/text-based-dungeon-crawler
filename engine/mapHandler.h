#ifndef MAP_HANDLER_H
#define MAP_HANDLER_H

#include <vector>
#include <iostream>
#include <random>


using namespace std;


class MapHandler {
    public:
        vector<vector<int>> grid;
        vector<pair<int, int>> enemyList;
        vector<vector<int>> bulletList;
        int currentLevel;

        explicit MapHandler(vector<vector<int>> grid);
        void update();

        pair<int, int> getPlayerPos();
        void movePlayer(int direction);
        int breakWall(int direction);

        bool isRunning;
        bool nextLvl;
        int score;
        
        
        void setMap(vector<vector<int>> newMap);

        

    private:
        random_device rd;
        mt19937 gen;

        int randint(int min, int max);
        void getEnemyAndBulletList();
        void updateEnemies();
        void updateBullets();
        int getTreasureCount();
        void executePowerUp(int powerup);
        void saveGridToJsonFile(string filename);


        bool isTimeStopped;
        bool isOmniDirectionActive;
        bool isXDirectionActive;
        bool isCrossDirectionActive;


        

        

};



#endif