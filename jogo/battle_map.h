#ifndef BATTLE_MAP_H
#define BATTLE_MAP_H

#include "raylib.h"

// Draws the battle map (floor, grid and marked tile)
void DrawBattleMap(int gridSizeX, int gridSizeZ, float tileWidth, float tileDepth, float offsetX, float offsetZ, int marcadoGX, int marcadoGZ);

// Converts grid coordinates to world coordinates (center of tile)
void GridToWorld(int gx, int gz, float tileWidth, float tileDepth, float offsetX, float offsetZ, float *outX, float *outZ);

#endif // BATTLE_MAP_H
