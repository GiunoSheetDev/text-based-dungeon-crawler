#include <windows.h>


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
    cin.ignore(1000, '\n');
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

void sendLevelMessage(HANDLE &hPipeLvl, const std::string &message) {
    if (hPipeLvl != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        BOOL success = WriteFile(
            hPipeLvl,
            message.c_str(),
            static_cast<DWORD>(message.size()),
            &bytesWritten,
            NULL
        );

        if (!success) {
            DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE) {
                std::cerr << "Python disconnected from level pipe.\n";
                CloseHandle(hPipeLvl);
                hPipeLvl = INVALID_HANDLE_VALUE;
            }
        }
    }
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

void handleInput(char input, MapHandler& handler, CaveGenerator& generator, HANDLE &hPipeLvl) {
    input = tolower(input);
    switch (input) {
        case 'w': handler.movePlayer(NORTH);  handler.score -= 1; break;
        case 's': handler.movePlayer(SOUTH); handler.score -= 1; break;
        case 'a': handler.movePlayer(WEST);  handler.score -= 1; break;
        case 'd': handler.movePlayer(EAST);  handler.score -= 1; break;
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
            sendLevelMessage(hPipeLvl, "newLvl");
            break;
        }
        default: break;
    }
}




int main() {
    // Register signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);

    CaveGenerator generator(50, 35);
    vector<vector<int>> initmap = generator.generateMap(1);
    MapHandler handler(initmap);
    g_handler = &handler;

    bool firstIteration = true;

    const char* INPUTPIPE = R"(\\.\pipe\game_input)";
    const char* LEVELPIPE = R"(\\.\pipe\game_level)";

    HANDLE hPipeIn = INVALID_HANDLE_VALUE;  // Python → C++
    HANDLE hPipeLvl = INVALID_HANDLE_VALUE; // C++ → Python

    try {
        while (handler.isRunning) {

            if (firstIteration) {
                firstIteration = false;
                vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                handler.setMap(newMap);
                handler.score = 0;

                // Notify Python of new level
                sendLevelMessage(hPipeLvl, "newLvl");
            }

            // (Re)create input pipe if disconnected
            if (hPipeIn == INVALID_HANDLE_VALUE) {
                hPipeIn = CreateNamedPipeA(
                    INPUTPIPE,
                    PIPE_ACCESS_INBOUND,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
                    1,
                    0,
                    0,
                    0,
                    NULL
                );
                if (hPipeIn == INVALID_HANDLE_VALUE) {
                    std::cerr << "Failed to create input pipe.\n";
                } else {
                    std::cout << "Input pipe ready.\n";
                }
            }

            // (Re)create level pipe if disconnected
            if (hPipeLvl == INVALID_HANDLE_VALUE) {
                hPipeLvl = CreateNamedPipeA(
                    LEVELPIPE,
                    PIPE_ACCESS_OUTBOUND,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
                    1,
                    0,
                    0,
                    0,
                    NULL
                );
                if (hPipeLvl == INVALID_HANDLE_VALUE) {
                    std::cerr << "Failed to create level pipe.\n";
                } else {
                    std::cout << "Level pipe ready.\n";
                }
            }

            if (handler.nextLvl) {
                vector<vector<int>> newMap = generator.generateMap(handler.currentLevel);
                handler.setMap(newMap);
                handler.nextLvl = false;

                // Notify Python of new level
                sendLevelMessage(hPipeLvl, "newLvl");
            }

            print(handler.grid);
            std::cout << "WASD to move, IJKL to dig / shoot. Spacebar to restart." << std::endl;
            handler.update();

            // --- Handle player input from keyboard ---
            if (_kbhit()) {
                char input = _getch();
                handleInput(input, handler, generator, hPipeLvl);
            }

            // --- Handle input from Python via input pipe ---
            if (hPipeIn != INVALID_HANDLE_VALUE) {
                char buffer[128];
                DWORD bytesRead = 0;
                BOOL success = ReadFile(hPipeIn, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
                if (success && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    for (DWORD i = 0; i < bytesRead; ++i) {
                        handleInput(buffer[i], handler, generator, hPipeLvl);
                    }
                } else if (!success) {
                    DWORD err = GetLastError();
                    if (err == ERROR_BROKEN_PIPE) {
                        std::cerr << "Python disconnected from input pipe.\n";
                        CloseHandle(hPipeIn);
                        hPipeIn = INVALID_HANDLE_VALUE;
                    }
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
