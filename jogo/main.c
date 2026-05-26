#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "battle_map.h"
#include "game_state.h"
#include "menu.h"
#include "ui.h"
#include "monster.h"
#include "card.h"

typedef struct {
    bool active;
    int player;
    int gx;
    int gz;
    int slot;
    MonsterSide chosenSide;
    float cursorT;
} MonsterPlacementChallenge;

int main(void)
{
    const int screenWidth = 1980;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Grid Movement");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 7.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 4.0f, 0.0f };
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
    AppScreen screen = APP_SCREEN_MAIN_MENU;

    DeckSetupState deckSetup;
    InitDeckSetupState(&deckSetup);

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

    MonsterAnimation monsterAnim = MonsterAnimationCreate();
    MonsterPlacementChallenge monsterPlacement = {0};
    
    int transformStage = 0;
    int transformTimer = 0;
    bool transforming = false;

    // =========================
    // MODELO DA CARTA
    // =========================

    Mesh cardMesh = GenMeshPlane(2.0f, 3.0f, 1, 1);

    Model cardModel = LoadModelFromMesh(cardMesh);

    ConfigureCardModel(&cardModel, playerTexture);

    // LOOP PRINCIPAL
    // =========================

    while (!WindowShouldClose())
    {
        if (screen == APP_SCREEN_MAIN_MENU)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                InitDeckSetupState(&deckSetup);
                screen = APP_SCREEN_DECK_SETUP;
            }

            if (IsKeyPressed(KEY_ESCAPE))
                break;

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawMainMenuScreen(screenWidth, screenHeight);
            EndDrawing();
            continue;
        }

        if (screen == APP_SCREEN_DECK_SETUP)
        {
            if (UpdateDeckSetupState(&deckSetup))
            {
                screen = APP_SCREEN_BATTLE;
                activePlayer = 0;
                playerGX = 2;
                playerGZ = 1;
                marcadoGX = -1;
                marcadoGZ = -1;
                player1Score = 0;
                player2Score = 0;
                placeMessage[0] = '\0';
                battleMessage[0] = '\0';
                battleResolveTimer = 0;
                battleResolveGX = -1;
                battleResolveGZ = -1;
                battleAwaitingActivation = false;
                battleActivatedByPlayer[0] = false;
                battleActivatedByPlayer[1] = false;
                battleWinnerOwner = -1;
                monsterPlacement.active = false;
                transforming = false;
                transformStage = 0;
                transformTimer = 0;
                placedFeedbackTimer = 0;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawDeckSetupScreen(screenWidth, screenHeight, &deckSetup);
            EndDrawing();
            continue;
        }

        // =========================
        // ANIMAÇÃO DO MONSTRO
        // =========================

        if (MonsterAnimationUpdate(&monsterAnim))
        {
            transforming = true;
            transformStage = 0;
            transformTimer = 0;
        }

        // =========================
        // TRANSFORMAÇÃO
        // =========================

        UpdateMonsterTransformation(&transforming, &transformTimer, &transformStage);

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
                TileEntity battleTile = GetTileAt(battleResolveGX, battleResolveGZ);
                MonsterPlacement m0 = battleTile.monsters[0];
                MonsterPlacement m1 = battleTile.monsters[1];

                int winnerOwner = -1;

                if (ResolveTileBattle(battleResolveGX, battleResolveGZ, &winnerOwner))
                {
                    if (winnerOwner == 0) player1Score++;
                    else if (winnerOwner == 1) player2Score++;

                    if (m0.owner == m1.owner)
                    {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d colocou 2 monstros e levou o ponto", winnerOwner + 1);
                    }
                    else if (m0.power > m1.power)
                    {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d venceu a batalha", m0.owner + 1);
                    }
                    else if (m1.power > m0.power)
                    {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d venceu a batalha", m1.owner + 1);
                    }
                    else
                    {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "Empate: carta decidiu");
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

        if (!monsterPlacement.active)
        {
            if (IsKeyPressed(KEY_S)) playerGX++;
            if (IsKeyPressed(KEY_W)) playerGX--;
            if (IsKeyPressed(KEY_D)) playerGZ--;
            if (IsKeyPressed(KEY_A)) playerGZ++;

            if (playerGX < 0) playerGX = 0;
            if (playerGZ < 0) playerGZ = 0;
            if (playerGX >= gridSizeX) playerGX = gridSizeX - 1;
            if (playerGZ >= gridSizeZ) playerGZ = gridSizeZ - 1;
        }

        // =========================
        // MARCAR TILE
        // =========================

        if (!monsterPlacement.active && IsKeyPressed(KEY_ENTER))
        {
            marcadoGX = playerGX;
            marcadoGZ = playerGZ;
        }

        if (!monsterPlacement.active && IsKeyPressed(KEY_ESCAPE))
        {
            marcadoGX = -1;
            marcadoGZ = -1;
        }

        // =========================
        // FEEDBACK TIMER
        // =========================

        if (placedFeedbackTimer > 0)
            placedFeedbackTimer--;

        if (monsterPlacement.active)
        {
            if (IsKeyPressed(KEY_A)) monsterPlacement.chosenSide = MONSTER_SIDE_LEFT;
            if (IsKeyPressed(KEY_D)) monsterPlacement.chosenSide = MONSTER_SIDE_RIGHT;

            monsterPlacement.cursorT += GetFrameTime() * 1.6f;
            if (monsterPlacement.cursorT > 1.0f)
                monsterPlacement.cursorT -= 1.0f;

            if (IsKeyPressed(KEY_SPACE))
            {
                MonsterSide hitSide = (monsterPlacement.cursorT < 0.5f)
                    ? MONSTER_SIDE_LEFT
                    : MONSTER_SIDE_RIGHT;

                MonsterSide finalSide =
                    (hitSide == monsterPlacement.chosenSide)
                    ? monsterPlacement.chosenSide
                    : (monsterPlacement.chosenSide == MONSTER_SIDE_LEFT
                        ? MONSTER_SIDE_RIGHT
                        : MONSTER_SIDE_LEFT);

                if (PlaceMonsterAt(
                        monsterPlacement.gx,
                        monsterPlacement.gz,
                        monsterPlacement.player,
                        monsterPlacement.slot,
                        deckSetup.monsterTypes[monsterPlacement.player][monsterPlacement.slot],
                        deckSetup.monsterElements[monsterPlacement.player][monsterPlacement.slot],
                        finalSide))
                {
                    float targetX, targetZ;
                    GridToWorld(
                        monsterPlacement.gx,
                        monsterPlacement.gz,
                        tileWidth,
                        tileDepth,
                        offsetX,
                        offsetZ,
                        &targetX,
                        &targetZ
                    );

                    int randomX = GetRandomValue(-10, 10);
                    int randomZ = GetRandomValue(-10, 10);

                    MonsterAnimationStart(
                        &monsterAnim,
                        finalSide,
                        (Vector3){ targetX + randomX, 4.0f, targetZ + randomZ },
                        (Vector3){ targetX, 0.7f, targetZ }
                    );

                    RemovePlayerMonsterFromHand(monsterPlacement.player, monsterPlacement.slot);

                    int monsterCount = GetTileMonsterCount(monsterPlacement.gx, monsterPlacement.gz);

                    if (monsterCount < 2)
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            (hitSide == monsterPlacement.chosenSide)
                                ? "Lado acertado"
                                : "Lado invertido"
                        );
                        placedFeedbackTimer = 60;
                    }
                    else
                    {
                        snprintf(battleMessage, sizeof(battleMessage) - 1, "Batalha!");
                        battleResolveGX = monsterPlacement.gx;
                        battleResolveGZ = monsterPlacement.gz;
                        battleAwaitingActivation = true;
                        battleActivatedByPlayer[0] = false;
                        battleActivatedByPlayer[1] = false;
                        battleWinnerOwner = -1;
                        PeekTileBattleWinner(battleResolveGX, battleResolveGZ, &battleWinnerOwner);
                    }

                    monsterPlacement.active = false;
                    marcadoGX = -1;
                    marcadoGZ = -1;
                    activePlayer ^= 1;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                monsterPlacement.active = false;
            }
        }

        // =========================
        // AÇÕES
        // =========================

        if (!monsterPlacement.active &&
            battleResolveTimer == 0 &&
            marcadoGX != -1 &&
            marcadoGZ != -1 &&
            !battleAwaitingActivation)
        {
            GetPlayerHands(playerCards, playerMonsters);

            bool canPlaceMonster = (CountPlayerCardsOnMap(activePlayer) > 0);

            if (IsKeyPressed(KEY_C))
            {
                for (int s = 0; s < 3; s++)
                {
                    if (playerCards[activePlayer][s])
                    {
                        if (PlaceCardAt(marcadoGX, marcadoGZ, activePlayer, s, deckSetup.cardTypes[activePlayer][s]))
                        {
                            RemovePlayerCardFromHand(activePlayer, s);
                            snprintf(placeMessage, sizeof(placeMessage) - 1, "Carta colocada");
                            placedFeedbackTimer = 60;
                            marcadoGX = -1;
                            marcadoGZ = -1;
                            activePlayer ^= 1;
                        }

                        break;
                    }
                }
            }

            if (IsKeyPressed(KEY_M))
            {
                if (!canPlaceMonster)
                {
                    strncpy(placeMessage, "Coloque uma carta primeiro!", sizeof(placeMessage) - 1);
                }
                else
                {
                    for (int s = 0; s < 3; s++)
                    {
                        if (playerMonsters[activePlayer][s])
                        {
                            monsterPlacement.active = true;
                            monsterPlacement.player = activePlayer;
                            monsterPlacement.gx = marcadoGX;
                            monsterPlacement.gz = marcadoGZ;
                            monsterPlacement.slot = s;
                            monsterPlacement.chosenSide = MONSTER_SIDE_LEFT;
                            monsterPlacement.cursorT = 0.0f;

                            snprintf(
                                placeMessage,
                                sizeof(placeMessage) - 1,
                                "A/D escolhe o lado | ESPACO confirma"
                            );

                            break;
                        }
                    }
                }
            }
        }

        // =========================
        // ATIVAÇÃO DE BATALHA
        // =========================

        if (battleAwaitingActivation)
        {
            if (IsKeyPressed(KEY_F))
            {
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

            TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
            bool needP0 = false;
            bool needP1 = false;

            if (bt.monsterCount >= 2)
            {
                needP0 = (bt.monsters[0].owner == 0) || (bt.monsters[1].owner == 0);
                needP1 = (bt.monsters[0].owner == 1) || (bt.monsters[1].owner == 1);
            }
            else if (bt.monsterCount == 1)
            {
                needP0 = (bt.monsters[0].owner == 0);
                needP1 = (bt.monsters[0].owner == 1);
            }

            bool ok = true;
            if (needP0 && !battleActivatedByPlayer[0]) ok = false;
            if (needP1 && !battleActivatedByPlayer[1]) ok = false;

            if (ok)
            {
                battleAwaitingActivation = false;
                battleResolveTimer = BATTLE_COUNTDOWN_FRAMES;
                snprintf(battleMessage, sizeof(battleMessage) - 1, "BATALHA: ativado!");
            }
        }

        if (IsKeyPressed(KEY_T))
            activePlayer ^= 1;

        // =========================
        // GRID -> MUNDO
        // =========================

        float playerX, playerZ;
        GridToWorld(playerGX, playerGZ, tileWidth, tileDepth, offsetX, offsetZ, &playerX, &playerZ);

        // =========================
        // DESENHO
        // =========================

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        DrawBattleMap(gridSizeX, gridSizeZ, tileWidth, tileDepth, offsetX, offsetZ, marcadoGX, marcadoGZ);

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

        Vector3 cardPos = { playerX, 0.5f, playerZ };
        DrawModel(cardModel, cardPos, 1.0f, WHITE);

        for (int gz = 0; gz < GetGridSizeZ(); gz++)
        {
            for (int gx = 0; gx < GetGridSizeX(); gx++)
            {
                TileEntity te = GetTileAt(gx, gz);
                float ex, ez;

                GridToWorld(gx, gz, tileWidth, tileDepth, offsetX, offsetZ, &ex, &ez);

                if (te.card.owner != -1)
                {
                    DrawCardModelAt(&cardModel, (Vector3){ ex, 0.03f, ez });

                    if (monsterPlacement.active &&
                        gx == monsterPlacement.gx &&
                        gz == monsterPlacement.gz)
                    {
                        DrawCube((Vector3){ ex, 0.05f, ez - 0.75f }, 1.8f, 0.03f, 1.35f, Fade(ORANGE, 0.85f));
                        DrawCube((Vector3){ ex, 0.05f, ez + 0.75f }, 1.8f, 0.03f, 1.35f, Fade(RED, 0.85f));
                        DrawLine3D((Vector3){ ex, 0.08f, ez - 1.5f }, (Vector3){ ex, 0.08f, ez + 1.5f }, BLACK);
                    }
                }

                if (te.monsterCount > 0)
                {
                    for (int m = 0; m < te.monsterCount; m++)
                    {
                        bool isBattleTile =
                            (battleResolveTimer > 0 &&
                             gx == battleResolveGX &&
                             gz == battleResolveGZ);

                        MonsterSide side = te.monsters[m].side;

                        float xOffset = 0.0f;
                        float zOffset = (side == MONSTER_SIDE_LEFT) ? -0.78f : 0.78f;
                        float yOffset = (side == MONSTER_SIDE_LEFT) ? 0.68f : 0.56f;

                        Vector3 monsterPos = {
                            ex + xOffset,
                            yOffset + (0.08f * m),
                            ez + zOffset
                        };

                        if (!monsterAnim.active)
                        {
                            float monsterScale = 0.8f;

                            if (isBattleTile)
                            {
                                if (te.monsters[m].owner == battleWinnerOwner && battleResolveTimer <= 30)
                                {
                                    float shakePhase = (float)(30 - battleResolveTimer);
                                    float shake = sinf(shakePhase * 10.0f) * 0.12f * (shakePhase / 30.0f);
                                    monsterPos.x += shake;
                                    monsterPos.z += shake * 0.5f;
                                }
                            }

                            DrawMonsterBillboard(
                                camera,
                                currentTexture,
                                monsterPos,
                                (Vector2){ monsterScale, monsterScale },
                                side
                            );
                        }
                    }
                }
            }
        }

        if (monsterAnim.active)
        {
            DrawMonsterBillboard(
                camera,
                currentTexture,
                monsterAnim.position,
                (Vector2){ 1.2f, 1.2f },
                monsterAnim.side
            );
        }

        EndMode3D();

        int hudY = 10;
        DrawScoreBoard(screenWidth, hudY, player1Score, player2Score, iconP1, iconP2);

        GetPlayerHands(playerCards, playerMonsters);

        DrawBottomMenu(screenWidth, screenHeight, activePlayer, playerCards, playerMonsters);

        if (battleAwaitingActivation && battleResolveGX != -1)
        {
            TileEntity bt = GetTileAt(battleResolveGX, battleResolveGZ);
            bool p0present = false;
            bool p1present = false;

            for (int m = 0; m < bt.monsterCount; m++)
            {
                if (bt.monsters[m].owner == 0) p0present = true;
                if (bt.monsters[m].owner == 1) p1present = true;
            }

            DrawBattleActivationPrompt(
                screenWidth,
                screenHeight,
                p0present,
                p1present,
                battleActivatedByPlayer[0],
                battleActivatedByPlayer[1]
            );
        }

        if (marcadoGX != -1 && marcadoGZ != -1)
        {
            bool canPickMonster = (CountPlayerCardsOnMap(activePlayer) > 0);
            DrawSelectionMenu(screenWidth, screenHeight, canPickMonster, activePlayer);
        }

        DrawBattleFeedbackOverlay(screenWidth, screenHeight, battleMessage, battleResolveTimer, placedFeedbackTimer);

        if (monsterPlacement.active)
        {
            DrawPlacementPrecisionBar(
                screenWidth,
                screenHeight,
                monsterPlacement.chosenSide,
                monsterPlacement.cursorT,
                monsterPlacement.player
            );

            DrawText(
                "Divida a carta em 2 lados e acerte a barra",
                screenWidth / 2 - 180,
                screenHeight - 210,
                16,
                BLACK
            );

            DrawText(
                "Lado esquerdo = primeira metade | lado direito = segunda metade",
                screenWidth / 2 - 230,
                screenHeight - 188,
                14,
                DARKGRAY
            );
        }
        DrawGameHints(screenWidth, screenHeight, placeMessage);

        EndDrawing();
    }

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