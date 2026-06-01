#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "battle_map.h"
#include "game_state.h"
#include "stats.h"
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

typedef enum {
    PLACEMENT_NONE = 0,
    PLACEMENT_CARD,
    PLACEMENT_MONSTER
} PlacementChoiceType;

typedef struct {
    bool active;
    PlacementChoiceType type;
    int player;
    int gx;
    int gz;
    int slot;
} PlacementChoiceState;

typedef struct {
    bool active;
    MonsterPlacement monster;
    int fromGX;
    int fromGZ;
    int toGX;
    int toGZ;
    Vector3 start;
    Vector3 target;
    Vector3 position;
    float t;
} CollisionDisplacementAnimation;

static int FindFirstAvailableSlot(const int slots[3])
{
    for (int slot = 0; slot < 3; slot++) {
        if (slots[slot]) return slot;
    }

    return -1;
}

static int FindNextAvailableSlot(const int slots[3], int currentSlot, int direction)
{
    if (currentSlot < 0) currentSlot = 0;

    for (int step = 1; step <= 3; step++) {
        int nextSlot = (currentSlot + direction * step + 3) % 3;
        if (slots[nextSlot]) return nextSlot;
    }

    return currentSlot;
}

static float Clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static Vector3 ComputeCardBlockTarget(float cardX, float cardZ, MonsterSide side, int stackIndex)
{
    float zOffset = (side == MONSTER_SIDE_LEFT) ? -0.78f : 0.78f;
    float yBase = (side == MONSTER_SIDE_LEFT) ? 0.68f : 0.56f;

    if (stackIndex < 0) stackIndex = 0;

    return (Vector3){
        cardX,
        yBase + (0.08f * (float)stackIndex),
        cardZ + zOffset
    };
}

static bool IsCardBlockNearEdge(Vector3 blockTarget, float offsetX, float offsetZ, float margin)
{
    return fabsf(blockTarget.x) >= (offsetX - margin) || fabsf(blockTarget.z) >= (offsetZ - margin);
}

static float ComputeLaunchPrecision(MonsterSide chosenSide, float cursorT)
{
    float center = (chosenSide == MONSTER_SIDE_LEFT) ? 0.25f : 0.75f;
    float distance = fabsf(cursorT - center);
    float normalized = Clamp01(distance / 0.25f);
    return 1.0f - normalized;
}

static float ComputeEdgeKnockoutChance(float precision)
{
    const float baseEdgeRisk = 0.10f;
    const float errorWeight = 0.55f;
    float chance = baseEdgeRisk + (1.0f - Clamp01(precision)) * errorWeight;
    if (chance > 0.95f) chance = 0.95f;
    return chance;
}

static bool RollChance(float chance)
{
    int roll = GetRandomValue(0, 1000);
    return roll < (int)(Clamp01(chance) * 1000.0f);
}

static float LerpFloat(float start, float end, float amount)
{
    return start + (end - start) * amount;
}

static Vector3 LerpVector3(Vector3 start, Vector3 end, float amount)
{
    return (Vector3){
        LerpFloat(start.x, end.x, amount),
        LerpFloat(start.y, end.y, amount),
        LerpFloat(start.z, end.z, amount)
    };
}

static bool BatalhaEmAndamento(bool battlePortalOpening, bool battleAwaitingActivation, int battleResolveTimer, int battleResolveGX, int battleResolveGZ)
{
    return battlePortalOpening ||
           battleAwaitingActivation ||
           battleResolveTimer > 0 ||
           (battleResolveGX != -1 && battleResolveGZ != -1);
}

static Vector3 ComputeKnockoutTarget(float x, float z, float offsetX, float offsetZ)
{
    const float outsideMargin = 1.6f;
    float leftDist = fabsf((-offsetX) - x);
    float rightDist = fabsf(offsetX - x);
    float topDist = fabsf((-offsetZ) - z);
    float bottomDist = fabsf(offsetZ - z);

    float minDist = leftDist;
    Vector3 out = (Vector3){ -offsetX - outsideMargin, 0.45f, z };

    if (rightDist < minDist) {
        minDist = rightDist;
        out = (Vector3){ offsetX + outsideMargin, 0.45f, z };
    }

    if (topDist < minDist) {
        minDist = topDist;
        out = (Vector3){ x, 0.45f, -offsetZ - outsideMargin };
    }

    if (bottomDist < minDist) {
        out = (Vector3){ x, 0.45f, offsetZ + outsideMargin };
    }

    return out;
}

static MonsterSide OppositeSide(MonsterSide side)
{
    return (side == MONSTER_SIDE_LEFT) ? MONSTER_SIDE_RIGHT : MONSTER_SIDE_LEFT;
}

static bool TileHasMonsterOnSide(TileEntity tile, MonsterSide side, MonsterPlacement *outMonster)
{
    for (int i = 0; i < tile.monsterCount; i++) {
        if (tile.monsters[i].side == side) {
            if (outMonster) *outMonster = tile.monsters[i];
            return true;
        }
    }

    return false;
}

static bool TryMoveCollidedMonsterToAdjacentCard(
    int sourceGX,
    int sourceGZ,
    int gridX,
    int gridZ,
    MonsterPlacement collided,
    int *outGX,
    int *outGZ)
{
    static const int dirs[4][2] = {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };

    for (int i = 0; i < 4; i++) {
        int nx = sourceGX + dirs[i][0];
        int nz = sourceGZ + dirs[i][1];

        if (nx < 0 || nz < 0 || nx >= gridX || nz >= gridZ) continue;
        if (!TileTemCarta(nx, nz)) continue;

        TileEntity neighbor = ObterTileEm(nx, nz);
        if (neighbor.monsterCount != 0) continue;

        if (ColocarMonstroEm(
                nx,
                nz,
                collided.owner,
                collided.slot,
                collided.type,
                collided.element,
                collided.side))
        {
            RemoverMonstroTilePorDonoSlot(sourceGX, sourceGZ, collided.owner, collided.slot);

            if (outGX) *outGX = nx;
            if (outGZ) *outGZ = nz;

            return true;
        }
    }

    return false;
}

static Vector3 ComputeMonsterWorldPosition(
    int gx,
    int gz,
    float tileWidth,
    float tileDepth,
    float offsetX,
    float offsetZ,
    MonsterSide side,
    int stackIndex)
{
    float x, z;

    GridToWorld(
        gx,
        gz,
        tileWidth,
        tileDepth,
        offsetX,
        offsetZ,
        &x,
        &z
    );

    float zOffset = (side == MONSTER_SIDE_LEFT) ? -0.78f : 0.78f;
    float yOffset = (side == MONSTER_SIDE_LEFT) ? 0.68f : 0.56f;

    return (Vector3){
        x,
        yOffset + (0.08f * stackIndex),
        z + zOffset
    };
}

static void IniciarAnimacaoColisao(
    CollisionDisplacementAnimation *anim,
    MonsterPlacement monster,
    int fromGX,
    int fromGZ,
    int toGX,
    int toGZ,
    Vector3 start,
    Vector3 target)
{
    if (!anim) return;

    anim->active = true;
    anim->monster = monster;
    anim->fromGX = fromGX;
    anim->fromGZ = fromGZ;
    anim->toGX = toGX;
    anim->toGZ = toGZ;
    anim->start = start;
    anim->target = target;
    anim->position = start;
    anim->t = 0.0f;
}

static void AtualizarAnimacaoColisao(CollisionDisplacementAnimation *anim)
{
    if (!anim || !anim->active) return;

    anim->t += GetFrameTime() * 2.6f;

    if (anim->t >= 1.0f)
    {
        anim->t = 1.0f;
        anim->active = false;
    }

    float t = anim->t;
    float smooth = t * t * (3.0f - 2.0f * t);
    float arc = sinf(t * 3.14159f) * 0.35f;

    anim->position.x = LerpFloat(anim->start.x, anim->target.x, smooth);
    anim->position.y = LerpFloat(anim->start.y, anim->target.y, smooth) + arc;
    anim->position.z = LerpFloat(anim->start.z, anim->target.z, smooth);
}

static bool DeveEsconderMonstroDeslocado(
    CollisionDisplacementAnimation *anim,
    int gx,
    int gz,
    MonsterPlacement monster)
{
    if (!anim || !anim->active) return false;

    return gx == anim->toGX &&
           gz == anim->toGZ &&
           monster.owner == anim->monster.owner &&
           monster.slot == anim->monster.slot;
}

static int ElementoPelaEscolhaDoMenu(int escolha)
{
    if (escolha <= 1) return BAKUGAN_ELEMENT_VENTO;
    if (escolha <= 3) return BAKUGAN_ELEMENT_AGUA;
    if (escolha <= 5) return BAKUGAN_ELEMENT_TERRA;
    if (escolha <= 7) return BAKUGAN_ELEMENT_FOGO;
    if (escolha <= 9) return BAKUGAN_ELEMENT_ESCURO;

    return BAKUGAN_ELEMENT_LUZ;
}

static int aEscolhaDoMenu(int escolha)
{
    if (escolha <= 1) return BAKUGAN_ELEMENT_VENTO;
    if (escolha <= 3) return BAKUGAN_ELEMENT_AGUA;
    if (escolha <= 5) return BAKUGAN_ELEMENT_TERRA;
    if (escolha <= 7) return BAKUGAN_ELEMENT_FOGO;
    if (escolha <= 9) return BAKUGAN_ELEMENT_ESCURO;

    return BAKUGAN_ELEMENT_LUZ;
}

static int BonusPorCartaPortal(int cardType)
{
    switch (cardType)
    {
        case CARD_TYPE_ATAQUE: return 18;
        case CARD_TYPE_DEFESA: return 14;
        case CARD_TYPE_ENERGIA: return 16;
        case CARD_TYPE_ARMADILHA: return 20;
        case CARD_TYPE_FOCO: return 12;
        case CARD_TYPE_SUPORTE: return 15;
        default: return 10;
    }
}

static int BonusPorTipoBakugan(int type)
{
    return 8 + ((type % 2) * 4) + ((type / 2) * 2);
}

static int BonusPorElementoBakugan(int element)
{
    static const int bonuses[BAKUGAN_ELEMENT_COUNT] = { 12, 14, 16, 13, 17, 19 };

    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
        return 0;

    return bonuses[element];
}

static Color CardRevealColor(int cardType)
{
    switch (cardType)
    {
        case CARD_TYPE_ATAQUE: return (Color){ 235, 125, 105, 255 };
        case CARD_TYPE_DEFESA: return (Color){ 115, 150, 235, 255 };
        case CARD_TYPE_ENERGIA: return (Color){ 240, 205, 100, 255 };
        case CARD_TYPE_ARMADILHA: return (Color){ 150, 110, 220, 255 };
        case CARD_TYPE_FOCO: return (Color){ 90, 190, 170, 255 };
        case CARD_TYPE_SUPORTE: return (Color){ 110, 205, 125, 255 };
        default: return RAYWHITE;
    }
}

static int CalcularPoderComPortal(CardPlacement portalCard, MonsterPlacement monster)
{
    // Cada monstro tem pontuação base fixa; a carta adiciona um bônus dependendo
    // do bronze selecionado e do elemento do monstro (aplicado apenas uma vez ao
    // ativar a carta portal).
    int base = BasePowerForType(monster.type);
    int cardBonus = CardBonusForPortalCard(portalCard.slot, portalCard.type, monster.element);
    return base + cardBonus;
}

static void AplicarStatusDaCartaPortal(int gx, int gz)
{
    TileEntity battleTile = ObterTileEm(gx, gz);

    if (battleTile.card.owner == -1)
        return;

    for (int i = 0; i < battleTile.monsterCount; i++)
    {
        MonsterPlacement monster = battleTile.monsters[i];
        int power = CalcularPoderComPortal(battleTile.card, monster);
        AtualizarPoderMonstroNoTile(gx, gz, monster.owner, monster.slot, power);
    }
}

static void DrawBattlePortalOverlay(int screenWidth, int screenHeight, TileEntity battleTile, bool portalOpening, int battlePortalTimer, int battlePortalTotalFrames)
{
    float progress = 1.0f;

    if (portalOpening && battlePortalTotalFrames > 0)
    {
        progress = 1.0f - ((float)battlePortalTimer / (float)battlePortalTotalFrames);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
    }

    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 6, 8, 14, 135 });

    int panelW = 760;
    int panelH = 286;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = 88;

    DrawRectangleRounded((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 0.12f, 12, (Color){ 20, 28, 44, 245 });
    DrawRectangleLinesEx((Rectangle){ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 2.0f, (Color){ 250, 212, 102, 220 });

    DrawText(portalOpening ? "CARTA PORTAL EM ABERTURA" : "CARTA PORTAL ABERTA", panelX + 24, panelY + 18, 28, (Color){ 255, 221, 117, 255 });
    DrawText(portalOpening ? "Aguarde o portal se abrir antes do duelo" : "Status aplicados. Pressione F/L para ativar", panelX + 24, panelY + 52, 18, RAYWHITE);

    float coreX = panelX + panelW * 0.5f;
    float coreY = panelY + 138.0f;
    float coreRadius = 42.0f + progress * 30.0f;

    DrawCircleV((Vector2){ coreX, coreY }, coreRadius, Fade(SKYBLUE, 0.6f));
    DrawCircleLines((int)coreX, (int)coreY, coreRadius + 8.0f, WHITE);

    float spread = 150.0f + progress * 70.0f;
    DrawRectangle(panelX + 156 - (int)spread, panelY + 92, 130, 92, Fade(ORANGE, 0.92f));
    DrawRectangle(panelX + 156 + (int)(progress * 8.0f), panelY + 92, 130, 92, Fade(RED, 0.92f));

    if (battleTile.monsterCount > 0)
    {
        for (int m = 0; m < battleTile.monsterCount && m < 2; m++)
        {
            int cardX = panelX + 410 + (m * 160);
            int cardY = panelY + 94;
            MonsterPlacement monster = battleTile.monsters[m];

            DrawRectangle(cardX, cardY, 138, 122, (Color){ 255, 255, 255, 20 });
            DrawRectangleLines(cardX, cardY, 138, 122, monster.owner == 0 ? ORANGE : RED);

            Texture2D portrait = ObterBakuganTexture(monster.type);
            DrawTextureEx(portrait, (Vector2){ (float)cardX + 8.0f, (float)cardY + 12.0f }, 0.0f, 0.32f, WHITE);

            DrawText(TextFormat("P%d", monster.owner + 1), cardX + 66, cardY + 12, 18, YELLOW);
            DrawText(BakuganTypeName(monster.type), cardX + 66, cardY + 34, 16, RAYWHITE);
            DrawText(BakuganElementName(monster.element), cardX + 66, cardY + 54, 16, SKYBLUE);
            DrawText(TextFormat("Poder %d", monster.power), cardX + 66, cardY + 78, 16, GOLD);
        }
    }
}

int main(void)
{
    const int screenWidth = 1880;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Movimento em Grade");
    SetExitKey(KEY_NULL);

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

    InicializarEstadoJogo(gridSizeX, gridSizeZ);

    int activePlayer = 0;
    TelaAplicacao screen = TELA_MENU_PRINCIPAL;

    EstadoPreparacaoDeck deckSetup;
    InicializarEstadoPreparacaoDeck(&deckSetup);

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
    bool battlePortalOpening = false;
    int battlePortalTimer = 0;
    const int BATTLE_PORTAL_FRAMES = 75;
    bool battlePortalStatusApplied = false;
    // batalha requer ativação pelos donos das cartas
    bool battleAwaitingActivation = false;
    bool battleActivatedByPlayer[2] = { false, false };
    const int BATTLE_COUNTDOWN_FRAMES = 180; // duração da batalha após ativações
    int battleWinnerOwner = -1;
    EstatisticasPartida estatisticasPartida = {0};
    int jogadorVencedor = -1;

    // =========================
    // TEXTURAS
    // =========================

    Texture2D playerTexture = LoadTexture("img/carta-base.png");
    Texture2D mapBackground = LoadTexture("jogo/img/fundo bakugan.png");
    
    
    CarregarMenu();
    CarregarAnimacoesBakugan();

    // =========================
    // ANIMAÇÃO
    // =========================
    MonsterAnimation monsterAnim = CriarAnimacaoMonstro();
    MonsterPlacementChallenge monsterPlacement = {0};
    PlacementChoiceState placementChoice = {0};
    CollisionDisplacementAnimation collisionAnim = {0};
    bool pendingKnockout = false;
    int pendingKnockoutOwner = -1;
    
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
        if (screen == TELA_MENU_PRINCIPAL)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                InicializarEstadoPreparacaoDeck(&deckSetup);
                ReiniciarMenu();
                ReiniciarProgressoPartida(&estatisticasPartida, &jogadorVencedor, &player1Score, &player2Score);

                screen = TELA_PREPARACAO_DECK;
            }

            if (IsKeyPressed(KEY_ESCAPE))
                break;

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DesenharTelaMenuPrincipal(screenWidth, screenHeight);
            EndDrawing();
            continue;
        }

        if (screen == TELA_PREPARACAO_DECK)
{
            AtualizarMenu();

            if (MenuFinalizado())
            {
                InicializarEstadoJogo(gridSizeX, gridSizeZ);

                for (int p = 0; p < 2; p++)
                {
                    deckSetup.cardTypes[p][0] = cartasBronzeEscolhidas[p];
                    deckSetup.cardTypes[p][1] = cartasPrataEscolhidas[p];
                    deckSetup.cardTypes[p][2] = cartasOuroEscolhidas[p];

                    for (int i = 0; i < 3; i++)
                    {
                        int escolha = bakugansEscolhidos[p][i];

                        deckSetup.monsterTypes[p][i] = escolha;
                        deckSetup.monsterElements[p][i] = ElementoPelaEscolhaDoMenu(escolha);
                    }
                }

                activePlayer = 0;

                playerGX = 2;
                playerGZ = 1;

                marcadoGX = -1;
                marcadoGZ = -1;

                player1Score = 0;
                player2Score = 0;

                battleResolveTimer = 0;
                battleResolveGX = -1;
                battleResolveGZ = -1;
                battlePortalOpening = false;
                battlePortalTimer = 0;
                battlePortalStatusApplied = false;
                battleAwaitingActivation = false;
                battleActivatedByPlayer[0] = false;
                battleActivatedByPlayer[1] = false;
                battleWinnerOwner = -1;

                monsterPlacement.active = false;
                placementChoice.active = false;
                placementChoice.type = PLACEMENT_NONE;
                placementChoice.slot = -1;

                pendingKnockout = false;
                pendingKnockoutOwner = -1;

                transforming = false;
                transformStage = 0;
                transformTimer = 0;
                monsterAnim.active = false;
                collisionAnim.active = false;

                placeMessage[0] = '\0';
                battleMessage[0] = '\0';
                placedFeedbackTimer = 0;

                screen = TELA_BATALHA;
            }
            

            BeginDrawing();

            DesenharMenu();

            EndDrawing();

            continue;
        }

        if (screen == TELA_VITORIA)
        {
            if (IsKeyPressed(KEY_S))
            {
                screen = TELA_ESTATISTICAS;
            }

            if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ENTER))
            {
                InicializarEstadoPreparacaoDeck(&deckSetup);
                ReiniciarProgressoPartida(&estatisticasPartida, &jogadorVencedor, &player1Score, &player2Score);
                screen = TELA_MENU_PRINCIPAL;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DesenharTelaVitoria(screenWidth, screenHeight, jogadorVencedor, player1Score, player2Score);
            EndDrawing();
            continue;
        }

        if (screen == TELA_ESTATISTICAS)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                screen = TELA_VITORIA;
            }

            if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ENTER))
            {
                InicializarEstadoPreparacaoDeck(&deckSetup);
                ReiniciarProgressoPartida(&estatisticasPartida, &jogadorVencedor, &player1Score, &player2Score);
                screen = TELA_MENU_PRINCIPAL;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DesenharTelaEstatisticas(screenWidth, screenHeight, &estatisticasPartida, player1Score, player2Score);
            EndDrawing();
            continue;
        }

        // =========================
        // ANIMAÇÃO DO MONSTRO
        // =========================

        if (AtualizarAnimacaoMonstro(&monsterAnim))
        {
            if (pendingKnockout)
            {
                if (pendingKnockoutOwner == 0)
                {
                    player1Score--;
                    if (player1Score < 0) player1Score = 0;
                }
                else if (pendingKnockoutOwner == 1)
                {
                    player2Score--;
                    if (player2Score < 0) player2Score = 0;
                }

                snprintf(placeMessage, sizeof(placeMessage) - 1, "Bakugan caiu para fora! P%d perdeu 1 ponto", pendingKnockoutOwner + 1);
                placedFeedbackTimer = 60;
                pendingKnockout = false;
                pendingKnockoutOwner = -1;
            }
            else
            {
                transforming = true;
                transformStage = 0;
                transformTimer = 0;
            }
        }

        // =========================
        // TRANSFORMAÇÃO
        // =========================

        AtualizarTransformacaoMonstro(&transforming, &transformTimer, &transformStage);
        AtualizarAnimacaoColisao(&collisionAnim);

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
                TileEntity battleTile = ObterTileEm(battleResolveGX, battleResolveGZ);
                MonsterPlacement m0 = battleTile.monsters[0];
                MonsterPlacement m1 = battleTile.monsters[1];

                int winnerOwner = -1;

                if (ResolverBatalhaNoTile(battleResolveGX, battleResolveGZ, &winnerOwner))
                {
                    if (winnerOwner == 0) player1Score++;
                    else if (winnerOwner == 1) player2Score++;

                    if (estatisticasPartida.quantidadeBatalhas < MAX_REGISTROS_BATALHA)
                    {
                        MontarRegistroBatalha(
                            &estatisticasPartida.registros[estatisticasPartida.quantidadeBatalhas],
                            estatisticasPartida.quantidadeBatalhas,
                            battleResolveGX,
                            battleResolveGZ,
                            battleTile,
                            winnerOwner
                        );
                        estatisticasPartida.quantidadeBatalhas++;
                    }

                    estatisticasPartida.jogadorVencedor = winnerOwner;
                    SalvarEstatisticasPartidaEmArquivo(&estatisticasPartida, player1Score, player2Score);

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

                    if (player1Score >= 3 || player2Score >= 3)
                    {
                        jogadorVencedor = (player1Score >= 3) ? 0 : 1;
                        estatisticasPartida.jogadorVencedor = jogadorVencedor;
                        SalvarEstatisticasPartidaEmArquivo(&estatisticasPartida, player1Score, player2Score);
                        battleResolveGX = -1;
                        battleResolveGZ = -1;
                        battleResolveTimer = 0;
                        battlePortalOpening = false;
                        battlePortalTimer = 0;
                        battlePortalStatusApplied = false;
                        battleAwaitingActivation = false;
                        battleActivatedByPlayer[0] = false;
                        battleActivatedByPlayer[1] = false;
                        battleWinnerOwner = -1;
                        monsterPlacement.active = false;
                        placementChoice.active = false;
                        placementChoice.type = PLACEMENT_NONE;
                        placementChoice.slot = -1;
                        pendingKnockout = false;
                        pendingKnockoutOwner = -1;
                        transforming = false;
                        transformStage = 0;
                        transformTimer = 0;
                        monsterAnim.active = false;
                collisionAnim.active = false;
                        screen = TELA_VITORIA;
                        continue;
                    }
                }

                battleResolveGX = -1;
                battleResolveGZ = -1;
                battleMessage[0] = '\0';
                battlePortalOpening = false;
                battlePortalTimer = 0;
                battlePortalStatusApplied = false;
            }
        }

        if (battlePortalOpening)
        {
            if (battlePortalTimer > 0)
                battlePortalTimer--;

            if (battlePortalTimer <= 0)
            {
                battlePortalOpening = false;

                if (!battlePortalStatusApplied && battleResolveGX != -1 && battleResolveGZ != -1)
                {
                    AplicarStatusDaCartaPortal(battleResolveGX, battleResolveGZ);
                    battlePortalStatusApplied = true;
                }

                if (battleResolveGX != -1 && battleResolveGZ != -1)
                {
                    VerificarVencedorBatalhaNoTile(battleResolveGX, battleResolveGZ, &battleWinnerOwner);
                }

                battleAwaitingActivation = true;
            }
        }

        // =========================
        // MOVIMENTO
        // =========================

        bool battleInProgress = BatalhaEmAndamento(battlePortalOpening, battleAwaitingActivation, battleResolveTimer, battleResolveGX, battleResolveGZ);

        if (!monsterPlacement.active && !placementChoice.active && !battleInProgress)
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

        if (!monsterPlacement.active && !placementChoice.active && !battleInProgress && IsKeyPressed(KEY_ENTER))
        {
            marcadoGX = playerGX;
            marcadoGZ = playerGZ;
        }

        if (!monsterPlacement.active && !placementChoice.active && !battleInProgress && IsKeyPressed(KEY_BACKSPACE))
        {
            marcadoGX = -1;
            marcadoGZ = -1;
        }

        // =========================
        // FEEDBACK TIMER
        // =========================

        if (placedFeedbackTimer > 0)
            placedFeedbackTimer--;

        ObterMaosJogadores(playerCards, playerMonsters);

        if (placementChoice.active)
        {
            const int *slots = (placementChoice.type == PLACEMENT_CARD)
                ? playerCards[placementChoice.player]
                : playerMonsters[placementChoice.player];

            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                placementChoice.slot = FindNextAvailableSlot(slots, placementChoice.slot, -1);
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                placementChoice.slot = FindNextAvailableSlot(slots, placementChoice.slot, 1);
            }

            for (int digit = 0; digit < 3; digit++) {
                if (IsKeyPressed(KEY_ONE + digit)) {
                    if (slots[digit]) {
                        placementChoice.slot = digit;
                    }
                    else {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "Slot indisponivel");
                        placedFeedbackTimer = 60;
                    }
                }
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                placementChoice.active = false;
                placementChoice.type = PLACEMENT_NONE;
                placementChoice.slot = -1;
                snprintf(placeMessage, sizeof(placeMessage) - 1, "Selecao cancelada");
                placedFeedbackTimer = 60;
            }
            else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (placementChoice.slot < 0 || !slots[placementChoice.slot]) {
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Escolha um slot disponivel");
                    placedFeedbackTimer = 60;
                }
                else if (placementChoice.type == PLACEMENT_CARD) {
                    if (ColocarCartaEm(placementChoice.gx, placementChoice.gz, placementChoice.player, placementChoice.slot, deckSetup.cardTypes[placementChoice.player][placementChoice.slot]))
                    {
                        RemoverCartaJogadorDaMao(placementChoice.player, placementChoice.slot);
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "Carta colocada");
                        placedFeedbackTimer = 60;
                        marcadoGX = -1;
                        marcadoGZ = -1;
                        activePlayer ^= 1;
                    }

                    placementChoice.active = false;
                    placementChoice.type = PLACEMENT_NONE;
                    placementChoice.slot = -1;
                }
                else if (placementChoice.type == PLACEMENT_MONSTER) {
                    monsterPlacement.active = true;
                    monsterPlacement.player = placementChoice.player;
                    monsterPlacement.gx = placementChoice.gx;
                    monsterPlacement.gz = placementChoice.gz;
                    monsterPlacement.slot = placementChoice.slot;
                    monsterPlacement.chosenSide = MONSTER_SIDE_LEFT;
                    monsterPlacement.cursorT = 0.0f;

                    snprintf(
                        placeMessage,
                        sizeof(placeMessage) - 1,
                        "A/D escolhe o lado | ESPACO confirma"
                    );

                    placementChoice.active = false;
                    placementChoice.type = PLACEMENT_NONE;
                    placementChoice.slot = -1;
                }
            }
        }

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
                float launchPrecision = ComputeLaunchPrecision(monsterPlacement.chosenSide, monsterPlacement.cursorT);
                bool collisionDisplaced = false;

                MonsterSide finalSide =
                    (hitSide == monsterPlacement.chosenSide)
                    ? monsterPlacement.chosenSide
                    : (monsterPlacement.chosenSide == MONSTER_SIDE_LEFT
                        ? MONSTER_SIDE_RIGHT
                        : MONSTER_SIDE_LEFT);

                TileEntity targetBeforePlacement = ObterTileEm(monsterPlacement.gx, monsterPlacement.gz);
                MonsterPlacement collidedMonster = EmptyMonsterPlacement();
                bool sideConflict = TileHasMonsterOnSide(targetBeforePlacement, finalSide, &collidedMonster);

                if (sideConflict)
                {
                    int displacedGX = -1;
                    int displacedGZ = -1;

                    if (TryMoveCollidedMonsterToAdjacentCard(
                            monsterPlacement.gx,
                            monsterPlacement.gz,
                            gridSizeX,
                            gridSizeZ,
                            collidedMonster,
                            &displacedGX,
                            &displacedGZ))
                    {
                        collisionDisplaced = true;

                        Vector3 startPos = ComputeMonsterWorldPosition(
                            monsterPlacement.gx,
                            monsterPlacement.gz,
                            tileWidth,
                            tileDepth,
                            offsetX,
                            offsetZ,
                            collidedMonster.side,
                            0
                        );

                        Vector3 targetPos = ComputeMonsterWorldPosition(
                            displacedGX,
                            displacedGZ,
                            tileWidth,
                            tileDepth,
                            offsetX,
                            offsetZ,
                            collidedMonster.side,
                            0
                        );

                        IniciarAnimacaoColisao(
                            &collisionAnim,
                            collidedMonster,
                            monsterPlacement.gx,
                            monsterPlacement.gz,
                            displacedGX,
                            displacedGZ,
                            startPos,
                            targetPos
                        );
                    }
                    else
                    {
                        MonsterSide alternateSide = OppositeSide(finalSide);
                        if (!TileHasMonsterOnSide(targetBeforePlacement, alternateSide, NULL))
                        {
                            finalSide = alternateSide;
                        }
                        else
                        {
                            snprintf(
                                placeMessage,
                                sizeof(placeMessage) - 1,
                                "Colisao sem carta adjacente livre para deslocamento"
                            );
                            placedFeedbackTimer = 60;
                            continue;
                        }
                    }
                }

                if (ColocarMonstroEm(
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

                    TileEntity placedTile = ObterTileEm(monsterPlacement.gx, monsterPlacement.gz);
                    int stackIndex = 0;
                    for (int i = 0; i < placedTile.monsterCount; i++) {
                        if (placedTile.monsters[i].owner == monsterPlacement.player &&
                            placedTile.monsters[i].slot == monsterPlacement.slot)
                        {
                            stackIndex = i;
                            break;
                        }
                    }

                    Vector3 cardBlockTarget = ComputeCardBlockTarget(targetX, targetZ, finalSide, stackIndex);
                    bool edgeBlock = IsCardBlockNearEdge(cardBlockTarget, offsetX, offsetZ, 0.25f);
                    float knockoutChance = ComputeEdgeKnockoutChance(launchPrecision);
                    bool knockedOut = edgeBlock && RollChance(knockoutChance);

                    Vector3 animationTarget = cardBlockTarget;
                    if (knockedOut)
                    {
                        animationTarget = ComputeKnockoutTarget(cardBlockTarget.x, cardBlockTarget.z, offsetX, offsetZ);
                    }

                    IniciarAnimacaoMonstro(
                        &monsterAnim,
                        finalSide,
                        (Vector3){ targetX + randomX, 4.0f, targetZ + randomZ },
                        animationTarget,
                        monsterPlacement.gx,
                        monsterPlacement.gz,
                        monsterPlacement.player,
                        monsterPlacement.slot,
                        deckSetup.monsterTypes[monsterPlacement.player][monsterPlacement.slot],
                        deckSetup.monsterElements[monsterPlacement.player][monsterPlacement.slot]
                    );

                    RemoverMonstroJogadorDaMao(monsterPlacement.player, monsterPlacement.slot);

                    if (knockedOut)
                    {
                        RemoverMonstroTilePorDonoSlot(
                            monsterPlacement.gx,
                            monsterPlacement.gz,
                            monsterPlacement.player,
                            monsterPlacement.slot
                        );
                        pendingKnockout = true;
                        pendingKnockoutOwner = monsterPlacement.player;
                        battleMessage[0] = '\0';
                        battleResolveGX = -1;
                        battleResolveGZ = -1;
                        battleResolveTimer = 0;
                        battleAwaitingActivation = false;
                        battleActivatedByPlayer[0] = false;
                        battleActivatedByPlayer[1] = false;
                        battleWinnerOwner = -1;
                    }

                    int monsterCount = ContarMonstrosNoTile(monsterPlacement.gx, monsterPlacement.gz);

                    if (knockedOut)
                    {
                        snprintf(placeMessage, sizeof(placeMessage) - 1, "Bakugan caiu para fora!");
                        placedFeedbackTimer = 60;
                    }
                    else if (collisionDisplaced)
                    {
                        snprintf(
                            placeMessage,
                            sizeof(placeMessage) - 1,
                            "Colisao! Bakugan atingido foi para carta ao lado"
                        );
                        placedFeedbackTimer = 60;
                    }
                    else if (monsterCount < 2)
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
                        battleResolveGX = monsterPlacement.gx;
                        battleResolveGZ = monsterPlacement.gz;
                        battlePortalOpening = true;
                        battlePortalTimer = BATTLE_PORTAL_FRAMES;
                        battlePortalStatusApplied = false;
                        battleAwaitingActivation = false;
                        battleActivatedByPlayer[0] = false;
                        battleActivatedByPlayer[1] = false;
                        battleWinnerOwner = -1;
                        snprintf(battleMessage, sizeof(battleMessage) - 1, "Carta portal abrindo...");
                    }

                    monsterPlacement.active = false;
                    marcadoGX = -1;
                    marcadoGZ = -1;
                    activePlayer ^= 1;
                }
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                monsterPlacement.active = false;
            }
        }

        // =========================
        // AÇÕES
        // =========================

        if (!monsterPlacement.active &&
            !placementChoice.active &&
            !battleInProgress &&
            marcadoGX != -1 &&
            marcadoGZ != -1)
        {
            bool canPlaceMonster = (ContarCartasJogadorNoMapa(activePlayer) > 0);

            if (IsKeyPressed(KEY_C))
            {
                int chosenSlot = FindFirstAvailableSlot(playerCards[activePlayer]);

                if (chosenSlot < 0) {
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Nao ha cartas disponiveis");
                    placedFeedbackTimer = 60;
                }
                else if (!PodeColocarCartaEm(marcadoGX, marcadoGZ)) {
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Nao pode colocar carta nesse tile");
                    placedFeedbackTimer = 60;
                }
                else {
                    placementChoice.active = true;
                    placementChoice.type = PLACEMENT_CARD;
                    placementChoice.player = activePlayer;
                    placementChoice.gx = marcadoGX;
                    placementChoice.gz = marcadoGZ;
                    placementChoice.slot = chosenSlot;
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Escolha o slot da carta");
                    placedFeedbackTimer = 60;
                }
            }

            if (IsKeyPressed(KEY_M))
            {
                int chosenSlot = FindFirstAvailableSlot(playerMonsters[activePlayer]);

                if (!canPlaceMonster)
                {
                    strncpy(placeMessage, "Coloque uma carta primeiro!", sizeof(placeMessage) - 1);
                    placedFeedbackTimer = 60;
                }
                else if (!TileTemCarta(marcadoGX, marcadoGZ))
                {
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Escolha um tile com carta");
                    placedFeedbackTimer = 60;
                }
                else if (chosenSlot < 0)
                {
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Nao ha bakugans disponiveis");
                    placedFeedbackTimer = 60;
                }
                else
                {
                    placementChoice.active = true;
                    placementChoice.type = PLACEMENT_MONSTER;
                    placementChoice.player = activePlayer;
                    placementChoice.gx = marcadoGX;
                    placementChoice.gz = marcadoGZ;
                    placementChoice.slot = chosenSlot;
                    snprintf(placeMessage, sizeof(placeMessage) - 1, "Escolha o bakugan do slot");
                    placedFeedbackTimer = 60;
                }
            }
        }
        else if (battleInProgress && (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_M)))
        {
            snprintf(placeMessage, sizeof(placeMessage) - 1, "Nao e possivel jogar durante a batalha");
            placedFeedbackTimer = 60;
        }

        // =========================
        // ATIVAÇÃO DE BATALHA
        // =========================

        if (battleAwaitingActivation)
        {
            if (IsKeyPressed(KEY_F))
            {
                TileEntity bt = ObterTileEm(battleResolveGX, battleResolveGZ);
                for (int m = 0; m < bt.monsterCount; m++)
                {
                    if (bt.monsters[m].owner == 0) { battleActivatedByPlayer[0] = true; break; }
                }
            }

            if (IsKeyPressed(KEY_L))
            {
                TileEntity bt = ObterTileEm(battleResolveGX, battleResolveGZ);
                for (int m = 0; m < bt.monsterCount; m++)
                {
                    if (bt.monsters[m].owner == 1) { battleActivatedByPlayer[1] = true; break; }
                }
            }

            TileEntity bt = ObterTileEm(battleResolveGX, battleResolveGZ);
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
        // FOCO DA CÂMERA
        // =========================

        Vector3 cameraBasePosition = { 10.0f, 7.0f, 0.0f };
        Vector3 cameraBaseTarget = { 0.0f, 0.0f, 0.0f };
        float cameraBaseFovy = 45.0f;

        Vector3 desiredCameraPosition = cameraBasePosition;
        Vector3 desiredCameraTarget = cameraBaseTarget;
        float desiredFovy = cameraBaseFovy;

        if (battleInProgress)
        {
            int focusGX = (battleResolveGX != -1) ? battleResolveGX : marcadoGX;
            int focusGZ = (battleResolveGZ != -1) ? battleResolveGZ : marcadoGZ;

            if (focusGX < 0) focusGX = playerGX;
            if (focusGZ < 0) focusGZ = playerGZ;

            float focusX, focusZ;
            GridToWorld(focusGX, focusGZ, tileWidth, tileDepth, offsetX, offsetZ, &focusX, &focusZ);

            desiredCameraTarget = (Vector3){ focusX, 0.7f, focusZ };
            desiredCameraPosition = (Vector3){ focusX + 4.0f, 5.6f, focusZ + 7.5f };
            desiredFovy = 38.0f;
        }

        camera.target = LerpVector3(camera.target, desiredCameraTarget, 0.08f);
        camera.position = LerpVector3(camera.position, desiredCameraPosition, 0.08f);
        camera.fovy = LerpFloat(camera.fovy, desiredFovy, 0.08f);

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
        if (screen == TELA_BATALHA)
        {
            DrawTexturePro(
                mapBackground,
                (Rectangle){ 0.0f, 0.0f, (float)mapBackground.width, (float)mapBackground.height },
                (Rectangle){ 0.0f, 0.0f, (float)screenWidth, (float)screenHeight },
                (Vector2){ 0.0f, 0.0f },
                0.0f,
                WHITE
            );
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, 40 });
        }
        BeginMode3D(camera);

        DrawBattleMap(gridSizeX, gridSizeZ, tileWidth, tileDepth, offsetX, offsetZ, marcadoGX, marcadoGZ, battleInProgress);

        
        if (!battleInProgress)
        {
            Vector3 cardPos = { playerX, 0.5f, playerZ };
            DrawModel(cardModel, cardPos, 1.0f, WHITE);
        }

        for (int gz = 0; gz < ObterTamanhoGridZ(); gz++)
        {
            for (int gx = 0; gx < ObterTamanhoGridX(); gx++)
            {
                TileEntity te = ObterTileEm(gx, gz);
                float ex, ez;

                GridToWorld(gx, gz, tileWidth, tileDepth, offsetX, offsetZ, &ex, &ez);

                if (te.card.owner != -1)
                {
                    bool battleTileActive = (gx == battleResolveGX && gz == battleResolveGZ && (battlePortalOpening || battleAwaitingActivation || battleResolveTimer > 0));

                    if (battleTileActive)
                    {
                        Texture2D cardTexture = ObterCartaTexturePorSlot(te.card.slot, te.card.type);

                        if (battlePortalOpening)
                        {
                            float flipProgress = 1.0f - ((float)battlePortalTimer / (float)BATTLE_PORTAL_FRAMES);
                            if (flipProgress < 0.0f) flipProgress = 0.0f;
                            if (flipProgress > 1.0f) flipProgress = 1.0f;

                            float cardAngle = 90.0f * (1.0f - flipProgress);
                            DrawCardModelExWithTexture(
                                &cardModel,
                                cardTexture,
                                (Vector3){ ex, 0.04f + (0.03f * flipProgress), ez },
                                (Vector3){ 0.0f, 1.0f, 0.0f },
                                cardAngle,
                                (Vector3){ 1.0f, 1.0f, 1.0f }
                            );
                        }
                        else
                        {
                            DrawCardModelAtWithTexture(&cardModel, cardTexture, (Vector3){ ex, 0.03f, ez });
                        }
                    }
                    else
                    {
                        DrawCardModelAtWithTexture(&cardModel, playerTexture, (Vector3){ ex, 0.03f, ez });
                    }

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
                        if (DeveEsconderMonstroDeslocado(&collisionAnim, gx, gz, te.monsters[m]))
                        {
                            continue;
                        }

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

                        // determine if this placed monster is the one currently animating
                        bool skipThisMonster = false;
                        if (monsterAnim.active && monsterAnim.gx == gx && monsterAnim.gz == gz)
                        {
                            if (monsterAnim.owner == te.monsters[m].owner && monsterAnim.slot == te.monsters[m].slot)
                                skipThisMonster = true;
                        }

                        if (!skipThisMonster)
                        {
                            float monsterScale = 1.2f;

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

                            Texture2D battleTexture = ObterBakuganTexture(te.monsters[m].type);
                            DesenharMonstroBillboard(
                                camera,
                                battleTexture,
                                monsterPos,
                                (Vector2){ monsterScale, monsterScale },
                                te.monsters[m].type,
                                side
                            );
                        }
                    }
                }
            }
        }

        if (collisionAnim.active)
        {
            Texture2D collisionTexture = ObterBakuganTexture(collisionAnim.monster.type);

            DesenharMonstroBillboard(
                camera,
                collisionTexture,
                collisionAnim.position,
                (Vector2){ 1.2f, 1.2f },
                collisionAnim.monster.type,
                collisionAnim.monster.side
            );
        }

        if (monsterAnim.active)
        {
            DesenharAnimacaoMonstroBillboard(
                camera,
                &monsterAnim,
                (Vector2){ 1.2f, 1.2f }
            );
        }

        EndMode3D();

        int hudY = 10;
        DrawScoreBoard(screenWidth, hudY, player1Score, player2Score, iconP1, iconP2);

        DrawBottomMenu(screenWidth, screenHeight, activePlayer, playerCards, playerMonsters);

        if ((battlePortalOpening || battleAwaitingActivation) && battleResolveGX != -1 && battleResolveGZ != -1)
        {
            TileEntity bt = ObterTileEm(battleResolveGX, battleResolveGZ);
            DrawMinimalBattleUI(screenWidth, screenHeight, battleResolveGX, battleResolveGZ, battlePortalOpening, battleAwaitingActivation, battlePortalStatusApplied, battlePortalTimer);

            DrawBattleCardPreview(
                screenWidth,
                screenHeight,
                bt.card.owner,
                bt.card.type,
                bt.card.slot,
                ObterCartaTexturePorSlot(bt.card.slot, bt.card.type),
                battlePortalOpening
            );
        }

        if (battleAwaitingActivation && battleResolveGX != -1)
        {
            TileEntity bt = ObterTileEm(battleResolveGX, battleResolveGZ);
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

        if (marcadoGX != -1 && marcadoGZ != -1 && !battleInProgress)
        {
            bool canPickMonster = (ContarCartasJogadorNoMapa(activePlayer) > 0);
            DrawChoicePanel(
                screenWidth,
                screenHeight,
                activePlayer,
                canPickMonster,
                false,
                false,
                NULL,
                -1,
                deckSetup.cardTypes[activePlayer],
                deckSetup.monsterTypes[activePlayer],
                deckSetup.monsterElements[activePlayer]
            );
        }

        if (placementChoice.active && !battleInProgress)
        {
            const int *slots = (placementChoice.type == PLACEMENT_CARD)
                ? playerCards[placementChoice.player]
                : playerMonsters[placementChoice.player];

            DrawChoicePanel(
                screenWidth,
                screenHeight,
                placementChoice.player,
                placementChoice.type == PLACEMENT_MONSTER,
                true,
                placementChoice.type == PLACEMENT_MONSTER,
                slots,
                placementChoice.slot,
                deckSetup.cardTypes[placementChoice.player],
                deckSetup.monsterTypes[placementChoice.player],
                deckSetup.monsterElements[placementChoice.player]
            );
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
    UnloadTexture(mapBackground);
    DescarregarAnimacoesBakugan();
    UnloadTexture(iconP1);
    UnloadTexture(iconP2);
    DescarregarMenu();

    LiberarEstadoJogo();
    CloseWindow();

    return 0;
}