#include "caveGenerator.h"
#include "mapHandler.h"

#include <iostream>
#include <limits>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <conio.h>
#include <string>




using namespace std;

#define FLOOR 0
#define WALL 1
#define TREASURE 2
#define ENEMY 3
#define PLAYER 4
#define BULLET_N 5
#define BULLET_S 6
#define BULLET_E 7
#define BULLET_W 8

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3



void print(const vector<vector<int>>& cellmap) {
    cout << "\033[H";  // Move cursor to top-left of the terminal
    cout << "\033[?25l";  // Hide cursor
    cout << flush;

    for (const vector<int>& row : cellmap) {
        for (int val : row) {
            switch (val) {
                case FLOOR:
                    cout << " . ";
                    break;
                case WALL:
                    cout << "\033[38;2;89;60;31m" << "***" << "\033[0m"; // brown
                    break;
                case TREASURE:
                    cout << "\033[33m" << " $ " << "\033[0m";  // Yellow
                    break;
                case ENEMY:
                    cout << "\033[31m" << " E " << "\033[0m"; // Red
                    break;
                case PLAYER:
                    cout << "\033[32m" << " P " << "\033[0m";  // Green
                    break;
                case BULLET_N:
                    cout << "\033[34m" << " o " << "\033[0m";  // Blue
                    break;
                case BULLET_S:
                    cout << "\033[34m" << " o " << "\033[0m";  // Blue
                    break;
                case BULLET_E:
                    cout << "\033[34m" << " o " << "\033[0m";  // Blue
                    break;
                case BULLET_W:
                    cout << "\033[34m" << " o " << "\033[0m";  // Blue
                    break;
            }
        }
        cout << endl;
    }
    cout << endl;
}




int main() {
    CaveGenerator generator(65, 45);
    
    
    vector<vector<int>> initmap = generator.generateMap(1);
    MapHandler handler(initmap);

    auto lastUpdate = chrono::steady_clock::now();


    try {
        while (handler.isRunning) {

            if (handler.nextLvl) {
                vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                handler.setMap(newMap);
                handler.nextLvl = false;
            }
            
            print(handler.map);
            cout << "WASD to move, IJKL to dig / shoot" << endl;
            handler.update();
            

            //handle player input
            if (_kbhit()) {
                char input = _getch();
            
                pair<int, int> playerPos = handler.getPlayerPos();

                switch (tolower(input)) {
                    case 'w': handler.movePlayer(NORTH);  handler.score -= 1; break;
                    case 's': handler.movePlayer(SOUTH); handler.score -= 1; break;
                    case 'a': handler.movePlayer(WEST); handler.score -= 1; break;
                    case 'd': handler.movePlayer(EAST); handler.score -= 1; break;
                    case 'i': {
                        int x = handler.breakWall(NORTH); 
                        handler.score -= (x == 0 ? 20 : 50);
                        break;}
                    case 'k': {
                        int x = handler.breakWall(SOUTH); 
                        handler.score -= (x == 0 ? 20 : 50);
                        break;}
                    case 'j':{
                        int x = handler.breakWall(WEST); 
                        handler.score -= (x == 0 ? 20 : 50);
                        break;}
                    case 'l': {
                        int x = handler.breakWall(EAST); 
                        handler.score -= (x == 0 ? 20 : 50);
                        break;}
                    default: break;
                }
            
            }

        
        }
    }
    catch (...) {
    // Silently ignore all exceptions

        system("cls");
        system("figlet \"GAME OVER\"");
        string scoreCommand = "figlet \"Score: " + to_string(handler.score) + "\"";
        system(scoreCommand.c_str());

        cout << "Press Enter to exit...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();

    }   


    system("cls");
    system("figlet \"GAME OVER\"");
    string scoreCommand = "figlet \"Score: " + to_string(handler.score) + "\"";
    system(scoreCommand.c_str());

    cout << "Press Enter to exit...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();




    



    return 0;
}