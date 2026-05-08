#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include "battle_map.h"
#include "game_state.h"
#include "ui.h"

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

    // Configuração do grid
    int gridSizeX= 4;
    int gridSizeZ= 2;
    float tileWidth = 2.0f;// eixo x
    float tileDepth = 3.0f; // eixo y
    float offsetX = (gridSizeX * tileWidth) / 2.0f;
    float offsetZ = (gridSizeZ * tileDepth) / 2.0f;

    float startX = -offsetX;
    float startZ = -offsetZ;

    // Jogador no grid
    int playerGX = 2;
    int playerGZ = 2;

    bool tileAtivo = false; // seleção de carta
    int marcadoGX = -1;
    int marcadoGZ = -1;

    // Placar (0..3)
    int player1Score = 0;
    int player2Score = 0;

    // Ícones - criar dinamicamente em vez de carregar arquivo
    int iconSize = 48;
    Image iconP1Img = GenImageColor(iconSize, iconSize, ORANGE);
    Image iconP2Img = GenImageColor(iconSize, iconSize, RED);
    Texture2D iconP1 = LoadTextureFromImage(iconP1Img);
    Texture2D iconP2 = LoadTextureFromImage(iconP2Img);
    UnloadImage(iconP1Img);
    UnloadImage(iconP2Img);

    // Inicializa o estado do jogo (mapa e mãos)
    InitGameState(gridSizeX, gridSizeZ);

    int activePlayer = 0; // 0 = P1, 1 = P2
    int playerCards[2][3];
    int playerMonsters[2][3];

    char placeMessage[128] = {0};
    int placedFeedbackTimer = 0; // temporizador de feedback visual
    char battleMessage[128] = {0};
    int battleResolveTimer = 0;
    int battleResolveGX = -1;
    int battleResolveGZ = -1;

    Texture2D playerTexture = LoadTexture("img/carta-base.png");
    Texture2D monsterTexture = LoadTexture("img/monster.png");
    Mesh cardMesh = GenMeshPlane(2.0f, 3.0f, 1, 1);

    Model cardModel = LoadModelFromMesh(cardMesh);

    cardModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = playerTexture;

    while (!WindowShouldClose())
    {
        if (battleResolveTimer > 0) {
            battleResolveTimer--;
            if (battleResolveTimer == 0 && battleResolveGX != -1 && battleResolveGZ != -1) {
                TileEntity battleTile = GetTileAt(battleResolveGX, battleResolveGZ);
                MonsterPlacement m0 = battleTile.monsters[0];
                MonsterPlacement m1 = battleTile.monsters[1];
                int winnerOwner = -1;

                if (ResolveTileBattle(battleResolveGX, battleResolveGZ, &winnerOwner)) {
                    if (winnerOwner == 0) player1Score++;
                    else if (winnerOwner == 1) player2Score++;

                    if (m0.owner == m1.owner) {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d colocou 2 monstros e levou o ponto", winnerOwner + 1);
                    } else if (m0.power > m1.power) {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d venceu a batalha e levou o ponto", m0.owner + 1);
                    } else if (m1.power > m0.power) {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "P%d venceu a batalha e levou o ponto", m1.owner + 1);
                    } else {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "Empate: ponto do dono da carta P%d", winnerOwner + 1);
                    }

                    placedFeedbackTimer = 60;
                }

                battleResolveGX = -1;
                battleResolveGZ = -1;
                battleMessage[0] = '\0';
            }
        }

        // ================= MOVIMENTO =================
        if (IsKeyPressed(KEY_D)) playerGX++;
        if (IsKeyPressed(KEY_A)) playerGX--;
        if (IsKeyPressed(KEY_W)) playerGZ--;
        if (IsKeyPressed(KEY_S)) playerGZ++;

        // Marcar tile
        if (IsKeyPressed(KEY_ENTER)){
            marcadoGX = playerGX;
            marcadoGZ = playerGZ;
        }

        // Desmarcar tile (ESC)
        if (IsKeyPressed(KEY_ESCAPE)) {
            marcadoGX = -1;
            marcadoGZ = -1;
        }

        // Atualiza o temporizador de feedback
        if (placedFeedbackTimer > 0) placedFeedbackTimer--;

        // Escolhe ação apenas se o tile estiver marcado
        if (battleResolveTimer == 0 && marcadoGX != -1 && marcadoGZ != -1) {
            GetPlayerHands(playerCards, playerMonsters);
            bool canPlaceMonster = (CountPlayerCardsOnMap(activePlayer) > 0);

            // C para colocar carta
            if (IsKeyPressed(KEY_C)) {
                if (!playerCards[activePlayer][0] && !playerCards[activePlayer][1] && !playerCards[activePlayer][2]) {
                    strncpy(placeMessage, "Sem cartas na mao", sizeof(placeMessage)-1);
                } else {
                    // Procura a primeira carta disponível
                    for (int s = 0; s < 3; s++) {
                        if (playerCards[activePlayer][s]) {
                            if (PlaceCardAt(marcadoGX, marcadoGZ, activePlayer, s)) {
                                RemovePlayerCardFromHand(activePlayer, s);
                                snprintf(placeMessage, sizeof(placeMessage)-1, "Colocado carta no tile (%d,%d)", marcadoGX, marcadoGZ);
                                placedFeedbackTimer = 60; // 1 segundo a 60 FPS
                                marcadoGX = marcadoGZ = -1;
                                activePlayer ^= 1; // passa a vez
                            } else {
                                if (TileHasCard(marcadoGX, marcadoGZ)) {
                                    strncpy(placeMessage, "Tile ja tem carta", sizeof(placeMessage)-1);
                                } else if (CountCardsOnMap() >= 4) {
                                    strncpy(placeMessage, "Limite de 4 cartas no tabuleiro atingido", sizeof(placeMessage)-1);
                                } else {
                                    strncpy(placeMessage, "Carta deve encostar em outra carta (lado/diagonal)", sizeof(placeMessage)-1);
                                }
                            }
                            break;
                        }
                    }
                }
            }

            // M para colocar monstro
            if (IsKeyPressed(KEY_M)) {
                if (!canPlaceMonster) {
                    strncpy(placeMessage, "Obrigatorio colocar carta primeiro!", sizeof(placeMessage)-1);
                } else if (!playerMonsters[activePlayer][0] && !playerMonsters[activePlayer][1] && !playerMonsters[activePlayer][2]) {
                    strncpy(placeMessage, "Sem monstros na mao", sizeof(placeMessage)-1);
                } else {
                    // Procura o primeiro monstro disponível
                    for (int s = 0; s < 3; s++) {
                        if (playerMonsters[activePlayer][s]) {
                            if (PlaceMonsterAt(marcadoGX, marcadoGZ, activePlayer, s)) {
                                RemovePlayerMonsterFromHand(activePlayer, s);
                                int monsterCount = GetTileMonsterCount(marcadoGX, marcadoGZ);
                                if (monsterCount < 2) {
                                    snprintf(placeMessage, sizeof(placeMessage)-1, "Monstro colocado no tile (%d,%d). Falta mais 1.", marcadoGX, marcadoGZ);
                                    placedFeedbackTimer = 60;
                                } else {
                                    TileEntity battleTile = GetTileAt(marcadoGX, marcadoGZ);
                                    MonsterPlacement m0 = battleTile.monsters[0];
                                    MonsterPlacement m1 = battleTile.monsters[1];

                                    if (m0.owner == m1.owner) {
                                        snprintf(battleMessage, sizeof(battleMessage)-1, "Batalha: 2 monstros do P%d", m0.owner + 1);
                                    } else if (m0.power > m1.power) {
                                        snprintf(battleMessage, sizeof(battleMessage)-1, "Batalha: P%d %d x %d P%d", m0.owner + 1, m0.power, m1.power, m1.owner + 1);
                                    } else if (m1.power > m0.power) {
                                        snprintf(battleMessage, sizeof(battleMessage)-1, "Batalha: P%d %d x %d P%d", m0.owner + 1, m0.power, m1.power, m1.owner + 1);
                                    } else {
                                        snprintf(battleMessage, sizeof(battleMessage)-1, "Batalha: empate, carta decide");
                                    }

                                    snprintf(placeMessage, sizeof(placeMessage)-1, "Batalha em andamento...");
                                    battleResolveGX = marcadoGX;
                                    battleResolveGZ = marcadoGZ;
                                    battleResolveTimer = 60;
                                }

                                marcadoGX = marcadoGZ = -1;
                                activePlayer ^= 1; // passa a vez
                            } else {
                                strncpy(placeMessage, "Tile sem carta ou ja tem monstro", sizeof(placeMessage)-1);
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Troca o jogador manualmente (apenas para teste)
        if (IsKeyPressed(KEY_T)) activePlayer ^= 1;

        // Limites do mapa
        if (playerGX < 0) playerGX = 0;
        if (playerGZ < 0) playerGZ = 0;
        if (playerGX >= gridSizeX) playerGX = gridSizeX - 1;
        if (playerGZ >= gridSizeZ) playerGZ = gridSizeZ - 1;

        // Converte grid → mundo (usa função do módulo)
        float playerX, playerZ;
        GridToWorld(playerGX, playerGZ, tileWidth, tileDepth, offsetX, offsetZ, &playerX, &playerZ);

        float markX = 0.0f, markZ = 0.0f;
        if (marcadoGX != -1 && marcadoGZ != -1) GridToWorld(marcadoGX, marcadoGZ, tileWidth, tileDepth, offsetX, offsetZ, &markX, &markZ);
        // ================= DESENHO =================
        BeginDrawing();
        ClearBackground(RAYWHITE);
        // teste 2D
        //DrawTexture(playerTexture, 100, 100, WHITE);

        BeginMode3D(camera);
        DrawBattleMap(gridSizeX, gridSizeZ, tileWidth, tileDepth, offsetX, offsetZ, marcadoGX, marcadoGZ);

        // posição da carta
        Vector3 cardPos = {playerX, 0.5f, playerZ};

        // desenha a carta deitada
        DrawModel(
            cardModel,
            cardPos,
            1.0f,
            WHITE
        );

        // Desenha as entidades colocadas no estado do jogo (cartas e monstros no mapa)
        for (int gz = 0; gz < GetGridSizeZ(); gz++) {
            for (int gx = 0; gx < GetGridSizeX(); gx++) {
                TileEntity te = GetTileAt(gx, gz);
                
                // Desenha a carta, se existir
                if (te.card.owner != -1) {
                    float ex, ez;
                    GridToWorld(gx, gz, tileWidth, tileDepth, offsetX, offsetZ, &ex, &ez);
                    Color cardCol = te.card.owner == 0 ? ORANGE : RED;
                    Vector3 cardPos = {ex, 0.1f, ez};
                    // Carta grande e bem visível no chão
                    DrawCube(cardPos, 1.8f, 0.05f, 2.4f, cardCol);
                    DrawCubeWires(cardPos, 1.8f, 0.05f, 2.4f, BLACK);
                }
                
                // Desenha o(s) monstro(s), se existir(em), acima da carta
                if (te.monsterCount > 0) {
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
                    for (int m = 0; m < te.monsterCount; m++) {
                        
                        
                        float xOffset =
                            (te.monsterCount == 2)
                            ? ((m == 0) ? -0.22f : 0.22f)
                            : 0.0f;

                        Vector3 monsterPos = {
                            ex + xOffset,
                            0.9f + (0.08f * m),
                            ez


                        
                        };


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

        EndMode3D();

        // HUD: ícones e barras de pontos (máximo 3)
        int hudY = 10;

        // Jogador 1 (esquerda)
        if (iconP1.id != 0) DrawTexture(iconP1, 10, hudY, WHITE);
        else DrawRectangle(10, hudY, 48, 48, MAROON);
        DrawText("P1", 10 + 48 + 6, hudY + 6, 18, BLACK);
        // Barras de pontos do P1
        for (int i = 0; i < 3; i++) {
            int bx = 10 + 48 + 6 + 40 + i * 22;
            int by = hudY + 6;
            DrawRectangleLines(bx, by, 18, 18, BLACK);
            if (i < player1Score) DrawRectangle(bx+1, by+1, 16, 16, GOLD);
        }

        // Jogador 2 (direita) - layout espelhado do P1
        int p2x = screenWidth - 10 - 48;
        int p2TextX = p2x - 36; // o texto fica à esquerda do ícone
        if (iconP2.id != 0) DrawTexture(iconP2, p2x, hudY, WHITE);
        else DrawRectangle(p2x, hudY, 48, 48, DARKBLUE);
        DrawText("P2", p2TextX, hudY + 6, 18, BLACK);
        // Barras de pontos do P2 (à esquerda do texto, espelhadas)
        for (int i = 0; i < 3; i++) {
            int bx = p2TextX - 40 - i * 22; // avança para a esquerda
            int by = hudY + 6;
            DrawRectangleLines(bx, by, 18, 18, BLACK);
            if (i < player2Score) DrawRectangle(bx+1, by+1, 16, 16, GOLD);
        }

        DrawText("WASD para mover (1 tile por vez)", 10, screenHeight - 30, 20, BLACK);

        // Desenha o menu inferior com os slots disponíveis
        GetPlayerHands(playerCards, playerMonsters);
        DrawBottomMenu(screenWidth, screenHeight, activePlayer, playerCards, playerMonsters);

        // Desenha o menu de seleção se o tile estiver marcado
        if (marcadoGX != -1 && marcadoGZ != -1) {
            bool canPickMonster = (CountPlayerCardsOnMap(activePlayer) > 0);
            DrawSelectionMenu(screenWidth, screenHeight, canPickMonster, activePlayer);
        }

        // Desenha o feedback visual da colocação
        if (battleResolveTimer > 0 && battleMessage[0] != '\0') {
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 35});
            DrawText(battleMessage, screenWidth/2 - MeasureText(battleMessage, 20)/2, 40, 20, YELLOW);
        }

        if (placedFeedbackTimer > 0) {
            float alpha = (placedFeedbackTimer / 60.0f);
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, (unsigned char)(50 * alpha)});
            int feedbackSize = 100 + (int)(50 * (1.0f - alpha));
            DrawRectangleLines(screenWidth/2 - feedbackSize/2, screenHeight/2 - feedbackSize/2, feedbackSize, feedbackSize, (Color){0, 255, 0, (unsigned char)(255 * alpha)});
        }

        // Instruções
        DrawText("ENTER marca tile, C coloca carta, M coloca monstro, ESC desmarca, T troca jogador", 10, screenHeight - 52, 14, DARKGRAY);
        if (placeMessage[0] != '\0') DrawText(placeMessage, 10, screenHeight - 72, 14, RED);

        EndDrawing();
    }
    
    UnloadModel(cardModel);
    UnloadTexture(playerTexture);
    UnloadTexture(monsterTexture);
    UnloadTexture(iconP1);
    UnloadTexture(iconP2);
    FreeGameState();
    CloseWindow();
    return 0;
}