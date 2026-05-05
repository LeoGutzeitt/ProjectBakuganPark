#include "raylib.h"

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "Grid Movement");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // CONFIG DO GRID
    int gridSize = 5;
    float tileSize = 1.0f;
    float offset = (gridSize * tileSize) / 2.0f;

    float startX = -offset;
    float startZ = -offset;

    // PLAYER NO GRID
    int playerGX = 2;
    int playerGZ = 2;

    while (!WindowShouldClose())
    {
        // ================= MOVIMENTO =================
        if (IsKeyPressed(KEY_D)) playerGX++;
        if (IsKeyPressed(KEY_A)) playerGX--;
        if (IsKeyPressed(KEY_W)) playerGZ--;
        if (IsKeyPressed(KEY_S)) playerGZ++;

        // LIMITES DO MAPA
        if (playerGX < 0) playerGX = 0;
        if (playerGZ < 0) playerGZ = 0;
        if (playerGX >= gridSize) playerGX = gridSize - 1;
        if (playerGZ >= gridSize) playerGZ = gridSize - 1;

        // CONVERTER GRID → MUNDO
        float playerX = startX + (playerGX + 0.5f) * tileSize;
        float playerZ = startZ + (playerGZ + 0.5f) * tileSize;

        // ================= DESENHO =================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // CHÃO
        DrawPlane((Vector3){0,0,0}, (Vector2){5,5}, GREEN);

        // GRID
        for (int i = 0; i <= gridSize; i++)
        {
            DrawLine3D(
                (Vector3){-offset, 0.01f, -offset + i * tileSize},
                (Vector3){ offset, 0.01f, -offset + i * tileSize},
                BLACK
            );

            DrawLine3D(
                (Vector3){-offset + i * tileSize, 0.01f, -offset},
                (Vector3){-offset + i * tileSize, 0.01f,  offset},
                BLACK
            );
        }

        // PLAYER (AGORA É O CUBO)
        Vector3 Playerpos3D = {playerX, 0.25f, playerZ};
        DrawCube(Playerpos3D, 1, 0.5, 1, RED);
        DrawCubeWires(Playerpos3D, 1.0f, 0.5f, 1.0f,BLACK);

        EndMode3D();

        DrawText("WASD para mover (1 tile por vez)", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}