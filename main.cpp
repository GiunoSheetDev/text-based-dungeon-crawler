#include "caveGenerator.h"
#include "mapHandler.h"
#include "picojson.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <conio.h>
#include <string>
#include <csignal>

using namespace std;

#include "tiletypes.h"

void saveVector(const vector<int>& v, const char* filename) {
    picojson::array arr;
    for (int x : v)
        arr.push_back(picojson::value((double)x));
    picojson::value val(arr);

    ofstream(filename) << val.serialize();
}

vector<int> loadVector(const char* filename) {
    ifstream ifs(filename);
    picojson::value val;
    picojson::parse(val, ifs);

    picojson::array arr = val.get<picojson::array>();
    vector<int> result;
    for (auto& v : arr)
        result.push_back((int)v.get<double>());

    return result;
}

// Forward declaration so signal handler can use it
MapHandler* g_handler = nullptr;
bool g_gameOverShown = false;

void showGameOver() {
    if (g_gameOverShown) return; // prevent double printing
    g_gameOverShown = true;

    vector<int> pastscores = loadVector("pastscores.json");
    pastscores.push_back(g_handler->score);
    sort(pastscores.begin(), pastscores.end());
    if (pastscores.size() > 10) {
        pastscores.erase(pastscores.begin(), pastscores.end()-1);
    }
    saveVector(pastscores, "pastscores.json");

    system("cls");
    system("figlet \"GAME OVER\"");
    if (g_handler) {
        string scoreCommand = "figlet \"Score: " + to_string(g_handler->score) + "\"";
        system(scoreCommand.c_str());
        string highScore = "figlet \"High Score: " + to_string(pastscores.back()) + "\"";
        system(highScore.c_str());
    }
    cout << "Press Enter to exit...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void signalHandler(int signum) {
    if (g_handler != nullptr) {
        g_handler->isRunning = false;  // stop main loop gracefully
    }
    showGameOver();

    // Restore default signal handler and re-raise signal to terminate properly
    std::signal(signum, SIG_DFL);
    std::raise(signum);
}

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
                case BULLET_S:
                case BULLET_E:
                case BULLET_W:
                case BULLET_NE:
                case BULLET_NW:
                case BULLET_SE:
                case BULLET_SW:
                    cout << "\033[34m" << " o " << "\033[0m";  // Blue
                    break;
                case POWERUP_STOPTIME:
                    cout << "\033[35m" << " % " << "\033[0m"; 
                    break;
                case POWERUP_OMNIDIRECTIONALBULLETS:
                    cout << "\033[35m" << " * " << "\033[0m"; 
                    break;
                case POWERUP_CROSSDIRECTIONALBULLETS:
                    cout << "\033[35m" << " + " << "\033[0m"; 
                        break;
                case POWERUP_XDIRECTIONALBULLETS:
                    cout << "\033[35m" << " x " << "\033[0m"; 
                    break;

            }       
        }
        cout << endl;
    }
    cout << endl;
}

int main() {
    // Register signal handlers for fatal errors and interruptions
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);

    CaveGenerator generator(65, 45);
    vector<vector<int>> initmap = generator.generateMap(1);
    MapHandler handler(initmap);
    g_handler = &handler; // allow signal handler to access score and control loop

    auto lastUpdate = chrono::steady_clock::now();

    bool firstIteration = true;


    try {
        while (handler.isRunning) {

            if (firstIteration) {
                firstIteration = false;
                vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                handler.setMap(newMap);
                handler.score = 0;
            }


            if (handler.nextLvl) {
                vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                handler.setMap(newMap);
                handler.nextLvl = false;
            }

            print(handler.grid);
            cout << "WASD to move, IJKL to dig / shoot. Spacebar to restart." << endl;
            handler.update();

            // handle player input
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
                        break;
                    }
                    case 'k': {
                        int x = handler.breakWall(SOUTH);
                        handler.score -= (x == 0 ? 20 : 50);
                        break;
                    }
                    case 'j': {
                        int x = handler.breakWall(WEST);
                        handler.score -= (x == 0 ? 20 : 50);
                        break;
                    }
                    case 'l': {
                        int x = handler.breakWall(EAST);
                        handler.score -= (x == 0 ? 20 : 50);
                        break;
                    }

                    case ' ': {
                        vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                        handler.setMap(newMap);
                        handler.score = 0;
                    }
                    default: break;
                }
            }
        }
    }
    catch (...) {
        system("cls");
        showGameOver();
        return 0;
    }
    system("cls");
    showGameOver();
    return 0;
}

