#include "raylib.h"
#include "battle_map.h"

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

    // PLACAR (0..3)
    int player1Score = 0;
    int player2Score = 0;

    // ÍCONES (coloque suas imagens em SpritesPersonagens/p1.png e p2.png)
    Texture2D iconP1 = LoadTexture("SpritesPersonagens/p1.png");
    Texture2D iconP2 = LoadTexture("SpritesPersonagens/p2.png");

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

        // CONVERTER GRID → MUNDO (usa função do módulo)
        float playerX, playerZ;
        GridToWorld(playerGX, playerGZ, tileWidth, tileDepth, offsetX, offsetZ, &playerX, &playerZ);

        float markX = 0.0f, markZ = 0.0f;
        if (marcadoGX != -1 && marcadoGZ != -1) GridToWorld(marcadoGX, marcadoGZ, tileWidth, tileDepth, offsetX, offsetZ, &markX, &markZ);
        // ================= DESENHO =================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawBattleMap(gridSizeX, gridSizeZ, tileWidth, tileDepth, offsetX, offsetZ, marcadoGX, marcadoGZ);

        // PLAYER (AGORA É O CUBO)
        Vector3 Playerpos3D = {playerX, 0.25f, playerZ};
        DrawCube(Playerpos3D, 2.0f, 0.1f, 3.0f, RED);
        DrawCubeWires(Playerpos3D, 2.0f, 0.1f, 3.0f, BLACK);

        EndMode3D();

        // HUD: ícones e barras de pontos (max 3)
        int hudY = 10;
        int iconSize = 48;

        // Player 1 (left)
        if (iconP1.id != 0) DrawTexture(iconP1, 10, hudY, WHITE);
        else DrawRectangle(10, hudY, iconSize, iconSize, MAROON);
        DrawText("P1", 10 + iconSize + 6, hudY + 6, 18, BLACK);
        // barras de pontos P1
        for (int i = 0; i < 3; i++) {
            int bx = 10 + iconSize + 6 + 40 + i * 22;
            int by = hudY + 6;
            DrawRectangleLines(bx, by, 18, 18, BLACK);
            if (i < player1Score) DrawRectangle(bx+1, by+1, 16, 16, GOLD);
        }

        // Player 2 (right) - layout espelhado de P1
        int p2x = screenWidth - 10 - iconSize;
        int p2TextX = p2x - 36; // texto fica à esquerda do ícone
        if (iconP2.id != 0) DrawTexture(iconP2, p2x, hudY, WHITE);
        else DrawRectangle(p2x, hudY, iconSize, iconSize, DARKBLUE);
        DrawText("P2", p2TextX, hudY + 6, 18, BLACK);
        // barras de pontos P2 (esquerda do texto, espelhadas)
        for (int i = 0; i < 3; i++) {
            int bx = p2TextX - 40 - i * 22; // avançar à esquerda
            int by = hudY + 6;
            DrawRectangleLines(bx, by, 18, 18, BLACK);
            if (i < player2Score) DrawRectangle(bx+1, by+1, 16, 16, GOLD);
        }

        DrawText("WASD para mover (1 tile por vez)", 10, screenHeight - 30, 20, BLACK);

        EndDrawing();
    }

    UnloadTexture(iconP1);
    UnloadTexture(iconP2);
    CloseWindow();
    return 0;
}