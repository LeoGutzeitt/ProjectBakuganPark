#include "raylib.h"

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define TILE_SIZE 64

// Mapa: 0 = chão, 1 = parede
int mapa[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,1},
    {1,1,1,0,0,0,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 640;

    InitWindow(screenWidth, screenHeight, "Mapa 2.5D - Raylib");

    Vector2 player = {100, 100};
    float speed = 3.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // ================= UPDATE =================

        Vector2 oldPos = player;

        if (IsKeyDown(KEY_D)) player.x += speed;
        if (IsKeyDown(KEY_A)) player.x -= speed;
        if (IsKeyDown(KEY_W)) player.y -= speed;
        if (IsKeyDown(KEY_S)) player.y += speed;

        // ================= COLISÃO =================

        int gridX = player.x / TILE_SIZE;
        int gridY = player.y / TILE_SIZE;

        if (mapa[gridY][gridX] == 1)
        {
            player = oldPos; // volta se bater na parede
        }

        // ================= DRAW =================

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // desenhar mapa
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                if (mapa[y][x] == 1)
                {
                    // parede
                    DrawRectangle(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);

                    // sombra fake (efeito 2.5D)
                    DrawRectangle(x*TILE_SIZE+5, y*TILE_SIZE+5, TILE_SIZE, TILE_SIZE, Fade(BLACK, 0.3f));
                }
                else
                {
                    // chão
                    DrawRectangle(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, LIGHTGRAY);
                }
            }
        }

        // player
        DrawCircleV(player, 20, BLUE);

        DrawText("WASD para mover", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}