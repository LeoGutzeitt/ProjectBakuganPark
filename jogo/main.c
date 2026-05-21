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
    camera.position = (Vector3){ 8.0f, 5.0f, 0.0f };
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
    // batalha requer ativação pelos donos das cartas
    bool battleAwaitingActivation = false;
    bool battleActivatedByPlayer[2] = { false, false };
    const int BATTLE_COUNTDOWN_FRAMES = 180; // duração da batalha após ativações
    int battleWinnerOwner = -1;

    // =========================
    // TEXTURAS
    // =========================

    Texture2D playerTexture = LoadTexture("img/carta-base.png");
    
    Texture2D monsterStage0 = LoadTexture("img/monster.png");
    Texture2D monsterStage1 = LoadTexture("img/monster1.png");
    Texture2D monsterStage2 = LoadTexture("img/monster2.png");

    // =========================
    // ANIMAÇÃO
    // =========================

    MonsterAnimation monsterAnim = {0};
    
    int transformStage = 0;
    int transformTimer = 0;
    bool transforming = false;

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

                transforming = true;
                transformStage = 0;
                transformTimer = 0;
            }
        }
    
        
        // =========================
        // TRANSFORMAÇÃO
        // =========================

        if (transforming)
        {
            transformTimer++;

            // após 60 frames -> estágio 1 (mais lento)
            if (transformTimer > 60)
            {
                transformStage = 1;
            }

            // após 120 frames -> estágio 2 (mais lento)
            if (transformTimer > 120)
            {
                transformStage = 2;
            }

            // termina animação
            if (transformTimer > 180)
            {
                transforming = false;
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

        if (IsKeyPressed(KEY_S)) playerGX++;
        if (IsKeyPressed(KEY_W)) playerGX--;
        if (IsKeyPressed(KEY_D)) playerGZ--;
        if (IsKeyPressed(KEY_A)) playerGZ++;

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
            marcadoGZ != -1 &&
            !battleAwaitingActivation)
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

                                    // enter activation phase: owners must press to activate their card
                                    battleAwaitingActivation = true;
                                    battleActivatedByPlayer[0] = false;
                                    battleActivatedByPlayer[1] = false;
                                    battleWinnerOwner = -1;
                                    PeekTileBattleWinner(battleResolveGX, battleResolveGZ, &battleWinnerOwner);
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
        // ATIVAÇÃO DE BATALHA (quando entrada de confirmação é necessária)
        // Player 0 pressiona F, Player 1 pressiona L
        if (battleAwaitingActivation)
        {
            // allow owner(s) to activate their card(s)
            if (IsKeyPressed(KEY_F))
            {
                // check if player 0 has a monster in that tile
                TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
                for (int m = 0; m < bt.monsterCount; m++)
                {
                    if (bt.monsters[m].owner == 0) { battleActivatedByPlayer[0] = true; break; }
                }
            }

            if (IsKeyPressed(KEY_L))
            {
                TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
                for (int m = 0; m < bt.monsterCount; m++)
                {
                    if (bt.monsters[m].owner == 1) { battleActivatedByPlayer[1] = true; break; }
                }
            }

            // determine if activation requirement satisfied:
            TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
            bool needP0 = false, needP1 = false;
            if (bt.monsterCount >= 2)
            {
                needP0 = (bt.monsters[0].owner == 0) || (bt.monsters[1].owner == 0);
                needP1 = (bt.monsters[0].owner == 1) || (bt.monsters[1].owner == 1);
            }
            else if (bt.monsterCount == 1)
            {
                // single monster: require its owner only
                needP0 = (bt.monsters[0].owner == 0);
                needP1 = (bt.monsters[0].owner == 1);
            }

            bool ok = true;
            if (needP0 && !battleActivatedByPlayer[0]) ok = false;
            if (needP1 && !battleActivatedByPlayer[1]) ok = false;

            if (ok)
            {
                // both (or single) activated -> start battle countdown
                battleAwaitingActivation = false;
                battleResolveTimer = BATTLE_COUNTDOWN_FRAMES;
                // small message
                snprintf(battleMessage, sizeof(battleMessage)-1, "BATALHA: ativado!");
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
        // ESCOLHE TEXTURA
        // =========================

        Texture2D currentTexture;

        if (transformStage == 0)
        {
            currentTexture = monsterStage0;
        }
        else if (transformStage == 1)
        {
            currentTexture = monsterStage1;
        }
        else
        {
            currentTexture = monsterStage2;
        }

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
                            float monsterScale = 0.8f;
                            float displayXOffset = xOffset;
                            float faceRotDeg = 0.0f;

                            if (battleResolveTimer > 0 && gx == battleResolveGX && gz == battleResolveGZ)
                            {
                                int owner = te.monsters[m].owner;
                                displayXOffset = (owner == 0) ? -0.35f : 0.35f;
                                faceRotDeg = (owner == 0) ? 0.0f : 180.0f;

                                if (owner == battleWinnerOwner && battleResolveTimer <= 30)
                                {
                                    float shakePhase = (float)(30 - battleResolveTimer);
                                    float shake = sinf(shakePhase * 10.0f) * 0.12f * (shakePhase / 30.0f);
                                    monsterPos.x += shake;
                                }
                            }

                            monsterPos.x = ex + displayXOffset;

                            Rectangle source = { 0, 0, (float)currentTexture.width, (float)currentTexture.height };
                            Vector2 origin = { 0.5f, 0.5f };

                            DrawBillboardPro(
                                camera,
                                currentTexture,
                                source,
                                monsterPos,
                                (Vector3){0.0f, 1.0f, 0.0f},
                                (Vector2){ monsterScale, monsterScale },
                                origin,
                                faceRotDeg,
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
                (float)currentTexture.width,
                (float)currentTexture.height
            };

            Vector2 origin = {
                0.6f,
                0.6f
            };

            DrawBillboardPro(
                camera,
                currentTexture,
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

        // Activation prompts during battle awaiting activation
        if (battleAwaitingActivation && battleResolveGX != -1)
        {
            TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
            int cy = screenHeight/2 - 40;
            int cx = screenWidth/2;
            // instructions
            DrawText("Ativar carta para iniciar batalha:", cx - 180, cy - 30, 18, BLACK);

            // Player 0 prompt
            bool p0present = false;
            bool p1present = false;
            for (int m = 0; m < bt.monsterCount; m++)
            {
                if (bt.monsters[m].owner == 0) p0present = true;
                if (bt.monsters[m].owner == 1) p1present = true;
            }

            if (p0present)
            {
                const char *s0 = battleActivatedByPlayer[0] ? "P1 ativado (F)" : "P1: pressione F";
                DrawText(s0, cx - 160, cy, 16, battleActivatedByPlayer[0] ? ORANGE : DARKGRAY);
            }
            if (p1present)
            {
                const char *s1 = battleActivatedByPlayer[1] ? "P2 ativado (L)" : "P2: pressione L";
                DrawText(s1, cx + 20, cy, 16, battleActivatedByPlayer[1] ? RED : DARKGRAY);
            }
        }

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
    UnloadTexture(monsterStage0);
    UnloadTexture(monsterStage1);
    UnloadTexture(monsterStage2);

    UnloadTexture(iconP1);
    UnloadTexture(iconP2);

    FreeGameState();

    CloseWindow();

    return 0;
}