#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "battle_map.h"
#include "game_state.h"
#include "ui.h"

typedef struct MonsterAnimation {
    Vector3 position;
    Vector3 target;
    float rotation;
    bool active;
} MonsterAnimation;

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "Grid Movement");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 8.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // =========================
    // CONFIGURAÇÃO DO GRID
    // =========================

    int gridSizeX = 4;
    int gridSizeZ = 2;

    float tileWidth = 2.0f;
    float tileDepth = 3.0f;

    float offsetX = (gridSizeX * tileWidth) / 2.0f;
    float offsetZ = (gridSizeZ * tileDepth) / 2.0f;

    // =========================
    // PLAYER
    // =========================

    int playerGX = 2;
    int playerGZ = 1;

    int marcadoGX = -1;
    int marcadoGZ = -1;

    // =========================
    // PLACAR
    // =========================

    int player1Score = 0;
    int player2Score = 0;

    // =========================
    // ICONES
    // =========================

    int iconSize = 48;

    Image iconP1Img = GenImageColor(iconSize, iconSize, ORANGE);
    Image iconP2Img = GenImageColor(iconSize, iconSize, RED);

    Texture2D iconP1 = LoadTextureFromImage(iconP1Img);
    Texture2D iconP2 = LoadTextureFromImage(iconP2Img);

    UnloadImage(iconP1Img);
    UnloadImage(iconP2Img);

    // =========================
    // GAME STATE
    // =========================

    InitGameState(gridSizeX, gridSizeZ);

    int activePlayer = 0;

    int playerCards[2][3];
    int playerMonsters[2][3];

    // =========================
    // MENSAGENS
    // =========================

    char placeMessage[128] = {0};
    int placedFeedbackTimer = 0;

    char battleMessage[128] = {0};

    int battleResolveTimer = 0;
    int battleResolveGX = -1;
    int battleResolveGZ = -1;

    // =========================
    // TEXTURAS
    // =========================

    Texture2D playerTexture = LoadTexture("img/carta-base.png");
    Texture2D monsterTexture = LoadTexture("img/monster.png");

    // =========================
    // ANIMAÇÃO
    // =========================

    MonsterAnimation monsterAnim = {0};

    // =========================
    // MODELO DA CARTA
    // =========================

    Mesh cardMesh = GenMeshPlane(2.0f, 3.0f, 1, 1);

    Model cardModel = LoadModelFromMesh(cardMesh);

    cardModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = playerTexture;

    // =========================
    // random
    // =========================

    int randX = GetRandomValue(-10, 10);
    int randZ = GetRandomValue(-10, 10);

    // =========================
    // LOOP PRINCIPAL
    // =========================



    while (!WindowShouldClose())
    {
        // =========================
        // ANIMAÇÃO DO MONSTRO
        // =========================

        if (monsterAnim.active)
        {
            float speed = 6.0f * GetFrameTime();

            monsterAnim.position.x +=
                (monsterAnim.target.x - monsterAnim.position.x) * speed;

            monsterAnim.position.y +=
                (monsterAnim.target.y - monsterAnim.position.y) * speed;

            monsterAnim.position.z +=
                (monsterAnim.target.z - monsterAnim.position.z) * speed;

            monsterAnim.rotation += 720.0f * GetFrameTime();

            float dx = monsterAnim.target.x - monsterAnim.position.x;
            float dy = monsterAnim.target.y - monsterAnim.position.y;
            float dz = monsterAnim.target.z - monsterAnim.position.z;

            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist < 0.05f)
            {
                monsterAnim.position = monsterAnim.target;
                monsterAnim.active = false;
                monsterAnim.rotation = 0.0f;
            }
        }

        // =========================
        // RESOLVE BATALHA
        // =========================

        if (battleResolveTimer > 0)
        {
            battleResolveTimer--;

            if (battleResolveTimer == 0 &&
                battleResolveGX != -1 &&
                battleResolveGZ != -1)
            {
                TileEntity battleTile =
                    GetTileAt(battleResolveGX, battleResolveGZ);

                MonsterPlacement m0 = battleTile.monsters[0];
                MonsterPlacement m1 = battleTile.monsters[1];

                int winnerOwner = -1;

                if (ResolveTileBattle(
                        battleResolveGX,
                        battleResolveGZ,
                        &winnerOwner))
                {
                    if (winnerOwner == 0) player1Score++;
                    else if (winnerOwner == 1) player2Score++;

                    if (m0.owner == m1.owner)
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            "P%d colocou 2 monstros e levou o ponto",
                            winnerOwner + 1
                        );
                    }
                    else if (m0.power > m1.power)
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            "P%d venceu a batalha",
                            m0.owner + 1
                        );
                    }
                    else if (m1.power > m0.power)
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            "P%d venceu a batalha",
                            m1.owner + 1
                        );
                    }
                    else
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            "Empate: carta decidiu"
                        );
                    }

                    placedFeedbackTimer = 60;
                }

                battleResolveGX = -1;
                battleResolveGZ = -1;

                battleMessage[0] = '\0';
            }
        }

        // =========================
        // MOVIMENTO
        // =========================

        if (IsKeyPressed(KEY_D)) playerGX++;
        if (IsKeyPressed(KEY_A)) playerGX--;
        if (IsKeyPressed(KEY_W)) playerGZ--;
        if (IsKeyPressed(KEY_S)) playerGZ++;

        // limites
        if (playerGX < 0) playerGX = 0;
        if (playerGZ < 0) playerGZ = 0;

        if (playerGX >= gridSizeX)
            playerGX = gridSizeX - 1;

        if (playerGZ >= gridSizeZ)
            playerGZ = gridSizeZ - 1;

        // =========================
        // MARCAR TILE
        // =========================

        if (IsKeyPressed(KEY_ENTER))
        {
            marcadoGX = playerGX;
            marcadoGZ = playerGZ;
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            marcadoGX = -1;
            marcadoGZ = -1;
        }

        // =========================
        // FEEDBACK TIMER
        // =========================

        if (placedFeedbackTimer > 0)
            placedFeedbackTimer--;

        // =========================
        // AÇÕES
        // =========================

        if (battleResolveTimer == 0 &&
            marcadoGX != -1 &&
            marcadoGZ != -1)
        {
            GetPlayerHands(playerCards, playerMonsters);

            bool canPlaceMonster =
                (CountPlayerCardsOnMap(activePlayer) > 0);

            // =====================
            // COLOCAR CARTA
            // =====================

            if (IsKeyPressed(KEY_C))
            {
                for (int s = 0; s < 3; s++)
                {
                    if (playerCards[activePlayer][s])
                    {
                        if (PlaceCardAt(
                                marcadoGX,
                                marcadoGZ,
                                activePlayer,
                                s))
                        {
                            RemovePlayerCardFromHand(activePlayer, s);

                            snprintf(
                                placeMessage,
                                sizeof(placeMessage)-1,
                                "Carta colocada"
                            );

                            placedFeedbackTimer = 60;

                            marcadoGX = -1;
                            marcadoGZ = -1;

                            activePlayer ^= 1;
                        }

                        break;
                    }
                }
            }

            // =====================
            // COLOCAR MONSTRO
            // =====================

            if (IsKeyPressed(KEY_M))
            {
                if (!canPlaceMonster)
                {
                    strncpy(
                        placeMessage,
                        "Coloque uma carta primeiro!",
                        sizeof(placeMessage)-1
                    );
                }
                else
                {
                    for (int s = 0; s < 3; s++)
                    {
                        if (playerMonsters[activePlayer][s])
                        {
                            if (PlaceMonsterAt(
                                    marcadoGX,
                                    marcadoGZ,
                                    activePlayer,
                                    s))
                            {
                                // =========================
                                // random
                                // =========================

                                int randX = GetRandomValue(-10, 10);
                                int randZ = GetRandomValue(-10, 10);

                                // posição do tile
                                float targetX, targetZ;

                                GridToWorld(
                                    marcadoGX,
                                    marcadoGZ,
                                    tileWidth,
                                    tileDepth,
                                    offsetX,
                                    offsetZ,
                                    &targetX,
                                    &targetZ
                                );

                                // animação
                                monsterAnim.position = (Vector3){
                                    targetX + randX,
                                    4.0f,
                                    targetZ + randZ
                                };

                                monsterAnim.target = (Vector3){
                                    targetX,
                                    0.7f,
                                    targetZ
                                };

                                monsterAnim.rotation = 0.0f;
                                monsterAnim.active = true;

                                RemovePlayerMonsterFromHand(
                                    activePlayer,
                                    s
                                );

                                int monsterCount =
                                    GetTileMonsterCount(
                                        marcadoGX,
                                        marcadoGZ
                                    );

                                if (monsterCount < 2)
                                {
                                    snprintf(
                                        placeMessage,
                                        sizeof(placeMessage)-1,
                                        "Monstro colocado"
                                    );

                                    placedFeedbackTimer = 60;
                                }
                                else
                                {
                                    snprintf(
                                        battleMessage,
                                        sizeof(battleMessage)-1,
                                        "Batalha!"
                                    );

                                    battleResolveGX = marcadoGX;
                                    battleResolveGZ = marcadoGZ;

                                    battleResolveTimer = 60;
                                }

                                marcadoGX = -1;
                                marcadoGZ = -1;

                                activePlayer ^= 1;
                            }

                            break;
                        }
                    }
                }
            }
        }

        // =========================
        // TROCA PLAYER
        // =========================

        if (IsKeyPressed(KEY_T))
            activePlayer ^= 1;

        // =========================
        // GRID -> MUNDO
        // =========================

        float playerX, playerZ;

        GridToWorld(
            playerGX,
            playerGZ,
            tileWidth,
            tileDepth,
            offsetX,
            offsetZ,
            &playerX,
            &playerZ
        );

        // =========================
        // DESENHO
        // =========================

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawBattleMap(
            gridSizeX,
            gridSizeZ,
            tileWidth,
            tileDepth,
            offsetX,
            offsetZ,
            marcadoGX,
            marcadoGZ
        );

        // =========================
        // PLAYER
        // =========================

        Vector3 cardPos = {
            playerX,
            0.5f,
            playerZ
        };

        DrawModel(
            cardModel,
            cardPos,
            1.0f,
            WHITE
        );

        // =========================
        // DESENHAR MAPA
        // =========================

        for (int gz = 0; gz < GetGridSizeZ(); gz++)
        {
            for (int gx = 0; gx < GetGridSizeX(); gx++)
            {
                TileEntity te = GetTileAt(gx, gz);

                float ex, ez;

                GridToWorld(
                    gx,
                    gz,
                    tileWidth,
                    tileDepth,
                    offsetX,
                    offsetZ,
                    &ex,
                    &ez
                );

                // =====================
                // CARTA
                // =====================

                if (te.card.owner != -1)
                {
                    Color cardCol =
                        te.card.owner == 0
                        ? ORANGE
                        : RED;

                    Vector3 tileCardPos = {
                        ex,
                        0.1f,
                        ez
                    };

                    DrawCube(
                        tileCardPos,
                        1.8f,
                        0.05f,
                        2.4f,
                        cardCol
                    );

                    DrawCubeWires(
                        tileCardPos,
                        1.8f,
                        0.05f,
                        2.4f,
                        BLACK
                    );
                }

                // =====================
                // MONSTROS
                // =====================

                if (te.monsterCount > 0)
                {
                    for (int m = 0; m < te.monsterCount; m++)
                    {
                        float xOffset =
                            (te.monsterCount == 2)
                            ? ((m == 0)
                                ? -0.22f
                                : 0.22f)
                            : 0.0f;

                        Vector3 monsterPos = {
                            ex + xOffset,
                            0.9f + (0.08f * m),
                            ez
                        };

                        // NÃO desenha o estático
                        // enquanto animação acontece

                        if (!monsterAnim.active)
                        {
                            DrawBillboard(
                                camera,
                                monsterTexture,
                                monsterPos,
                                1.2f,
                                WHITE
                            );
                        }
                    }
                }
            }
        }

        // =========================
        // MONSTRO ANIMADO
        // =========================

        if (monsterAnim.active)
        {
            Rectangle source = {
                0,
                0,
                (float)monsterTexture.width,
                (float)monsterTexture.height
            };

            Vector2 origin = {
                0.6f,
                0.6f
            };

            DrawBillboardPro(
                camera,
                monsterTexture,
                source,
                monsterAnim.position,
                (Vector3){0.0f, 1.0f, 0.0f},
                (Vector2){1.2f, 1.2f},
                origin,
                monsterAnim.rotation,
                WHITE
            );
        }

        EndMode3D();

        // =========================
        // HUD
        // =========================

        int hudY = 10;

        // P1

        DrawTexture(iconP1, 10, hudY, WHITE);

        DrawText(
            "P1",
            64,
            hudY + 6,
            18,
            BLACK
        );

        for (int i = 0; i < 3; i++)
        {
            int bx = 104 + i * 22;

            DrawRectangleLines(
                bx,
                hudY + 6,
                18,
                18,
                BLACK
            );

            if (i < player1Score)
            {
                DrawRectangle(
                    bx + 1,
                    hudY + 7,
                    16,
                    16,
                    GOLD
                );
            }
        }

        // P2

        int p2x = screenWidth - 58;

        DrawTexture(iconP2, p2x, hudY, WHITE);

        DrawText(
            "P2",
            p2x - 36,
            hudY + 6,
            18,
            BLACK
        );

        // =========================
        // UI
        // =========================

        GetPlayerHands(
            playerCards,
            playerMonsters
        );

        DrawBottomMenu(
            screenWidth,
            screenHeight,
            activePlayer,
            playerCards,
            playerMonsters
        );

        if (marcadoGX != -1 &&
            marcadoGZ != -1)
        {
            bool canPickMonster =
                (CountPlayerCardsOnMap(activePlayer) > 0);

            DrawSelectionMenu(
                screenWidth,
                screenHeight,
                canPickMonster,
                activePlayer
            );
        }

        // =========================
        // FEEDBACKS
        // =========================

        if (battleResolveTimer > 0 &&
            battleMessage[0] != '\0')
        {
            DrawRectangle(
                0,
                0,
                screenWidth,
                screenHeight,
                (Color){0,0,0,35}
            );

            DrawText(
                battleMessage,
                screenWidth/2 -
                MeasureText(battleMessage, 20)/2,
                40,
                20,
                YELLOW
            );
        }

        if (placedFeedbackTimer > 0)
        {
            float alpha =
                (placedFeedbackTimer / 60.0f);

            DrawRectangle(
                0,
                0,
                screenWidth,
                screenHeight,
                (Color){
                    0,
                    0,
                    0,
                    (unsigned char)(50 * alpha)
                }
            );
        }

        // =========================
        // TEXTO
        // =========================

        DrawText(
            "ENTER marca tile | C carta | M monstro",
            10,
            screenHeight - 52,
            14,
            DARKGRAY
        );

        if (placeMessage[0] != '\0')
        {
            DrawText(
                placeMessage,
                10,
                screenHeight - 72,
                14,
                RED
            );
        }

        EndDrawing();
    }

    // =========================
    // FINALIZAÇÃO
    // =========================

    UnloadModel(cardModel);

    UnloadTexture(playerTexture);
    UnloadTexture(monsterTexture);

    UnloadTexture(iconP1);
    UnloadTexture(iconP2);

    FreeGameState();

    CloseWindow();

    return 0;
}