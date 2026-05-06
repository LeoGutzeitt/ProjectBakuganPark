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
    int gridSizeX= 4;
    int gridSizeZ= 2;
    float tileWidth = 2.0f;//eixo x
    float tileDepth = 3.0f; //eixo y
    float offsetX = (gridSizeX * tileWidth) / 2.0f;
    float offsetZ = (gridSizeZ * tileDepth) / 2.0f;

    float startX = -offsetX;
    float startZ = -offsetZ;

    // PLAYER NO GRID
    int playerGX = 2;
    int playerGZ = 2;

    bool tileAtivo = false; //seleção de carta
    int marcadoGX = -1;
    int marcadoGZ = -1;

    while (!WindowShouldClose())
    {
        // ================= MOVIMENTO =================
        if (IsKeyPressed(KEY_D)) playerGX++;
        if (IsKeyPressed(KEY_A)) playerGX--;
        if (IsKeyPressed(KEY_W)) playerGZ--;
        if (IsKeyPressed(KEY_S)) playerGZ++;
        if (IsKeyPressed(KEY_ENTER)){
            
            marcadoGX = playerGX;
            marcadoGZ = playerGZ;
        }

        // LIMITES DO MAPA
        if (playerGX < 0) playerGX = 0;
        if (playerGZ < 0) playerGZ = 0;
        if (playerGX >= gridSizeX) playerGX = gridSizeX - 1;
        if (playerGZ >= gridSizeZ) playerGZ = gridSizeZ - 1;

        // CONVERTER GRID → MUNDO
        float playerX = startX + (playerGX + 0.5f) * tileWidth;
        float playerZ = startZ + (playerGZ + 0.5f) * tileDepth;


        float markX = startX + (marcadoGX + 0.5f) * tileWidth;
        float markZ = startZ + (marcadoGZ + 0.5f) * tileDepth;
        // ================= DESENHO =================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // CHÃO
        DrawPlane((Vector3){0,0,0}, (Vector2){8,6}, GREEN);
        DrawPlane((Vector3){0,0,3.5}, (Vector2){8,1}, DARKGRAY);
        DrawPlane((Vector3){0,0,-3.5}, (Vector2){8,1}, DARKGRAY);


        if (marcadoGX != -1)
        {
            DrawCube(
                (Vector3){markX, 0.02f, markZ},
                tileWidth, 0.02f, tileDepth,
                BLUE
            );
        }
        

        // GRID
        for (int i = 0; i <= gridSizeX; i++)
        {
            DrawLine3D(
                (Vector3){-offsetX, 0.01f, -offsetZ + i * tileDepth},
                (Vector3){ offsetX, 0.01f, -offsetZ + i * tileDepth},
                BLACK
            );

            DrawLine3D(
                (Vector3){-offsetX + i * tileWidth, 0.01f, -offsetZ},
                (Vector3){-offsetX + i * tileWidth, 0.01f,  offsetZ},
                BLACK
            );
        }

        // PLAYER (AGORA É O CUBO)
        Vector3 Playerpos3D = {playerX, 0.25f, playerZ};
        DrawCube(Playerpos3D, 2, 0.1, 3, RED);
        DrawCubeWires(Playerpos3D, 2.0f, 0.1f, 3.0f,BLACK);

        EndMode3D();

        DrawText("WASD para mover (1 tile por vez)", 10, 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}