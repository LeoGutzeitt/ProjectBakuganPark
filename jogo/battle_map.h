#ifndef BATTLE_MAP_H
#define BATTLE_MAP_H

#include "raylib.h"

// Desenha o mapa de batalha (chão, grid e tile marcado)
void DrawBattleMap(int gridSizeX, int gridSizeZ, float tileWidth, float tileDepth, float offsetX, float offsetZ, int marcadoGX, int marcadoGZ);

// Converte coordenadas do grid para coordenadas do mundo (centro do tile)
void GridToWorld(int gx, int gz, float tileWidth, float tileDepth, float offsetX, float offsetZ, float *outX, float *outZ);

#endif // fim de BATTLE_MAP_H
