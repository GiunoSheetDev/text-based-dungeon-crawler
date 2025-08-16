from tiletypes import Tile

import pygame #for the timers
import json
import time
import os
import heapq
import math


import pywintypes
import win32pipe
import win32file

from collections import defaultdict

pygame.init()



class Agent:
    def __init__(self, resolution: tuple[int, int]):
        self.width, self.heigth = resolution
    
        self.view = list[list[int]]
        self.viewList : list[list[list[int]]] = []
        self.viewUpdateTime : int = 0
        self.viewUpdateCooldown : int = 100
        self.viewUpdateCount : int = 1
        script_dir = os.path.dirname(os.path.abspath(__file__))  # folder of agent.py
        self.viewPath = os.path.abspath(os.path.join(script_dir, "..", "engine", "grid.json"))

        self.mapPlayerPos : tuple[int, int] = None
        self.mapEnemyPos : list[tuple[int, int]] = [] 
        self.mapTreasurePos : list[tuple[int, int]]= []
        self.mapPowerUpPos : list[tuple[int, int]]= []
        self.mapFloorPos : list[tuple[int, int]]= []
        self.mapWallPos : list[tuple[int, int]]= []
        self.mapBulletPos : list[tuple[int, int]]= []


        self.pathDict = defaultdict(dict)
        self.pathRoute = []
        self.pathCurrent = []


        self.inputpipe = r'\\.\pipe\game_input'
        self.levelpipe = r'\\.\pipe\game_level'


    def loadGrid(self, path: str) -> None:
        
        if pygame.time.get_ticks() - self.viewUpdateTime > self.viewUpdateCooldown:
            with open(path, "r") as file:
                if not os.path.exists(path) or os.path.getsize(path) == 0:
                    return
                
                self.view = json.load(file)
                self.collapseGrid()
                self.viewUpdateTime = pygame.time.get_ticks()
            
    def collapseGrid(self) -> None:
        for y in range(len(self.view)):
            for x in range(len(self.view[0])):
                cell = self.view[y][x]

                match cell:
                    case 0:  # floor
                        self.mapFloorPos.append((y, x))
                    case 1:  # walls
                        self.mapWallPos.append((y, x))
                    case 2:  # treasure
                        self.mapTreasurePos.append((y, x))
                    case 3:  # enemy
                        self.mapEnemyPos.append((y, x))
                    case 4:  # player
                        self.mapPlayerPos = (y, x)
                    case 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12:  # bullets
                        self.view[y][x] = Tile.BULLET
                        self.mapBulletPos.append((y, x))
                    case 13 | 14 | 15 | 16:  # powerups
                        self.view[y][x] = Tile.POWERUP
                        self.mapPowerUpPos.append((y, x))

    def tileCost(self, y, x) -> int:
        cell = self.view[y][x]
        match cell:
            case 0: 
                return 2
            case 1:
                return 20
            case 2:
                return 0
            case 3:
                return math.inf
            case 4:
                return math.inf
            case 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12:
                return math.inf
            case 13 | 14 | 15 | 16:
                return 1


    def _dijkstraCore(self, start: tuple[int,int], goal: tuple[int,int]):
        """Internal Dijkstra function, returns both dist and prev grids."""
        rows, cols = len(self.view), len(self.view[0])
        dist = [[math.inf] * cols for _ in range(rows)]
        prev = [[None] * cols for _ in range(rows)]

        sy, sx = start
        gy, gx = goal
        dist[sy][sx] = 0
        pq = [(0, sy, sx)]

        while pq:
            currentCost, y, x = heapq.heappop(pq)

            if (y, x) == (gy, gx):
                break

            if currentCost > dist[y][x]:
                continue

            for dy, dx in [(-1,0),(1,0),(0,-1),(0,1)]:
                ny, nx = y+dy, x+dx
                if not (0 <= ny < rows and 0 <= nx < cols):
                    continue

                cost = self.tileCost(ny, nx)
                if cost == math.inf:
                    continue

                newCost = currentCost + cost
                if newCost < dist[ny][nx]:
                    dist[ny][nx] = newCost
                    prev[ny][nx] = (y, x)
                    heapq.heappush(pq, (newCost, ny, nx))

        return dist, prev

    def dijkstraPath(self, start: tuple[int,int], goal: tuple[int,int]):
        """Compute and store only the path from start to goal."""
        dist, prev = self._dijkstraCore(start, goal)
        path = self.reconstructPath(prev, start, goal)
        path.pop(0)

        return path

    def dijkstraDistance(self, start: tuple[int,int], goal: tuple[int,int]):
        """Compute and store only the distance from start to goal."""
        dist, _ = self._dijkstraCore(start, goal)

        if start not in self.pathDict:
            self.pathDict[start] = {}
        self.pathDict[start][goal] = dist[goal[0]][goal[1]]

    def reconstructPath(self, prev, start, goal) -> list[tuple[int, int]]:
        path = []
        current = goal
        while current != start:
            if current is None:
                # No path found
                return []
            path.append(current)
            current = prev[current[0]][current[1]]
        path.append(start)  # add the start at the end
        path.reverse()
        return path

    def calculateDistances(self) -> None:
        for treasureStart in self.mapTreasurePos:
            self.dijkstraDistance(self.mapPlayerPos, treasureStart)
            for treasureEnd in self.mapTreasurePos:
                if treasureStart == treasureEnd:
                    continue

                self.dijkstraDistance(treasureStart, treasureEnd)

    def calculateRoute(self) -> None:
        self.pathRoute = []

        currentPos = self.mapPlayerPos
        nextPos = None
        while len(self.pathRoute) != len(self.mapTreasurePos):
            sortedPos = dict(sorted(self.pathDict[currentPos].items(), key=lambda item: item[1]))
            # Pick the first treasure not yet visited
            for pos in sortedPos.keys():
                if pos not in self.pathRoute:
                    nextPos = pos
                    break
                        
            self.pathRoute.append(nextPos)
            currentPos = nextPos

    def calculateNextMove(self) -> str:
        try:
            
            self.pathCurrent = self.dijkstraPath(self.mapPlayerPos, self.pathRoute[0])

            ny, nx = self.pathCurrent[0]

            nextCell = self.view[ny][nx]
            delta = (nextCell, ny- self.mapPlayerPos[0], nx-self.mapPlayerPos[1])

            match delta:
                #floor / powerup / treasure
                case (0, -1, 0) | (2, -1, 0) | (6, -1, 0):
                    return "w"
                case (0, 1, 0) | (2, 1, 0) | (6, 1, 0):
                    return "s"
                case (0, 0, 1) | (2, 0, 1) | (6, 0, 1):
                    return "d"        
                case (0, 0, -1) | (2, 0, -1) | (6, 0, -1):
                    return "a"
                

                #wall
                case (1, -1, 0):
                    return "i"
                case (1, 1, 0):
                    return "k"
                case (1, 0, 1):
                    return "l"
                case (1, 0, -1):
                    return "j"
                
                case _:
                    return " "
        except:
            return "z"


    def run(self) -> None:
        firstIter = True
        inputHandle = None
        levelHandle = None

        while True:
            self.loadGrid(self.viewPath)
            time.sleep(0.05)

            # (Re)connect to input pipe (write to C++)
            if inputHandle is None:
                try:
                    inputHandle = win32file.CreateFile(
                        self.inputpipe,  # \\.\pipe\game_input
                        win32file.GENERIC_WRITE,
                        0,
                        None,
                        win32file.OPEN_EXISTING,
                        0,
                        None
                    )
                    
                except pywintypes.error:
                    time.sleep(0.1)

            # (Re)connect to level pipe (read from C++)
            if levelHandle is None:
                try:
                    levelHandle = win32file.CreateFile(
                        self.levelpipe,  # \\.\pipe\game_level
                        win32file.GENERIC_READ,
                        0,
                        None,
                        win32file.OPEN_EXISTING,
                        0,
                        None
                    )
                    
                except pywintypes.error:
                    time.sleep(0.1)

            # Read from level pipe
            if levelHandle is not None:
                try:
                    _, data = win32file.ReadFile(levelHandle, 4096)
                    message = data.decode("utf-8").strip()
                    if message == "newLvl":
                        firstIter = True
                except pywintypes.error:
                    levelHandle = None

            # If first iteration or after newLvl message
            if firstIter:
                firstIter = False
                self.calculateDistances()
                self.calculateRoute()

            # Calculate next move and send to C++
            if inputHandle is not None:
                nextMove = self.calculateNextMove()
                try:
                    win32file.WriteFile(inputHandle, nextMove.encode("utf-8"))
                except pywintypes.error:
                    print("Input pipe disconnected.")
                    inputHandle = None
        
                




if __name__ == "__main__":
    a = Agent((1920, 1080))
    a.run()