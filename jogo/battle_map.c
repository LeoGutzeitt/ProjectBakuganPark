#include "battle_map.h"

void GridToWorld(int gx, int gz, float tileWidth, float tileDepth, float offsetX, float offsetZ, float *outX, float *outZ)
{
    float startX = -offsetX;
    float startZ = -offsetZ;
    if (outX) *outX = startX + (gx + 0.5f) * tileWidth;
    if (outZ) *outZ = startZ + (gz + 0.5f) * tileDepth;
}

void DrawBattleMap(int gridSizeX, int gridSizeZ, float tileWidth, float tileDepth, float offsetX, float offsetZ, int marcadoGX, int marcadoGZ)
{
    // CHÃO
    DrawPlane((Vector3){0,0,0}, (Vector2){8,6}, GREEN);
    DrawPlane((Vector3){0,0,3.5f}, (Vector2){8,1}, DARKGRAY);
    DrawPlane((Vector3){0,0,-3.5f}, (Vector2){8,1}, DARKGRAY);

    // MARCAÇÃO
    if (marcadoGX != -1 && marcadoGZ != -1)
    {
        float markX, markZ;
        GridToWorld(marcadoGX, marcadoGZ, tileWidth, tileDepth, offsetX, offsetZ, &markX, &markZ);
        DrawCube((Vector3){markX, 0.02f, markZ}, tileWidth, 0.02f, tileDepth, BLUE);
    }

    // GRID
    for (int i = 0; i <= gridSizeZ; i++)
    {
        DrawLine3D(
            (Vector3){-offsetX, 0.01f, -offsetZ + i * tileDepth},
            (Vector3){ offsetX, 0.01f, -offsetZ + i * tileDepth},
            BLACK
        );
    }

    for (int i = 0; i <= gridSizeX; i++)
    {
        DrawLine3D(
            (Vector3){-offsetX + i * tileWidth, 0.01f, -offsetZ},
            (Vector3){-offsetX + i * tileWidth, 0.01f,  offsetZ},
            BLACK
        );
    }
}
