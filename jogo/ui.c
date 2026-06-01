#include "ui.h"
#include "menu.h"
#include "game_state.h"
#include "card.h"
#include "monster.h"
#include <math.h>

static Color CardTypeColor(int type)
{
    switch (type)
    {
        case CARD_TYPE_ATAQUE: return (Color){ 230, 110, 90, 255 };
        case CARD_TYPE_DEFESA: return (Color){ 120, 150, 230, 255 };
        case CARD_TYPE_ENERGIA: return (Color){ 240, 205, 95, 255 };
        case CARD_TYPE_ARMADILHA: return (Color){ 140, 105, 210, 255 };
        case CARD_TYPE_FOCO: return (Color){ 95, 185, 170, 255 };
        case CARD_TYPE_SUPORTE: return (Color){ 105, 200, 120, 255 };
        default: return LIGHTGRAY;
    }
}

static void DrawMiniCardPreview(int x, int y, int w, int h, int cardType, bool selected, bool available)
{
    Color fill = available ? CardTypeColor(cardType) : (Color){ 180, 180, 180, 255 };
    Rectangle box = { (float)x, (float)y, (float)w, (float)h };

    DrawRectangleRec(box, Fade(fill, selected ? 1.0f : 0.86f));
    DrawRectangleLinesEx(box, selected ? 3.0f : 2.0f, selected ? BLACK : Fade(BLACK, 0.6f));
    DrawRectangle(x + 8, y + 8, w - 16, h - 16, Fade(WHITE, 0.18f));
    DrawText(CardTypeName(cardType), x + 12, y + h / 2 - 8, 16, BLACK);
}

static void DrawMiniMonsterPreview(int x, int y, int w, int h, int type, int element, bool selected, bool available)
{
    Rectangle box = { (float)x, (float)y, (float)w, (float)h };
    Color border = available ? (selected ? BLACK : (Color){ 255, 255, 255, 90 }) : (Color){ 130, 130, 130, 255 };

    DrawRectangleRec(box, available ? (Color){ 225, 230, 235, 255 } : (Color){ 185, 185, 185, 255 });
    DrawRectangleLinesEx(box, selected ? 3.0f : 2.0f, border);

    Texture2D texture = ObterBakuganTexture(type);
    DrawTextureEx(texture, (Vector2){ x + 10.0f, y + 8.0f }, 0.0f, 0.32f, WHITE);
    DrawText(BakuganTypeName(type), x + 76, y + 12, 15, BLACK);
    DrawText(BakuganElementName(element), x + 76, y + 34, 14, DARKBLUE);
    DrawText(TextFormat("Base %d", BasePowerForType(type)), x + 76, y + 54, 13, MAROON);
    DrawText(available ? "Disponivel" : "Usado", x + 76, y + 70, 13, available ? DARKGREEN : DARKGRAY);
}

static Color GetTurnColor(int activePlayer)
{
    return (activePlayer == 0) ? BLUE : RED;
}

static const char *CardBonusElementLabel(int displayIndex)
{
    static const char *labels[6] = { "Fogo", "Agua", "Terra", "Luz", "Escuro", "Vento" };

    if (displayIndex < 0 || displayIndex >= 6) return "?";
    return labels[displayIndex];
}

static int CardBonusElementToMonsterElement(int displayIndex)
{
    static const int mapping[6] = {
        BAKUGAN_ELEMENT_FOGO,
        BAKUGAN_ELEMENT_AGUA,
        BAKUGAN_ELEMENT_TERRA,
        BAKUGAN_ELEMENT_LUZ,
        BAKUGAN_ELEMENT_ESCURO,
        BAKUGAN_ELEMENT_VENTO
    };

    if (displayIndex < 0 || displayIndex >= 6) return BAKUGAN_ELEMENT_FOGO;
    return mapping[displayIndex];
}

static void DrawCardBonusDetailPanel(int x, int y, int w, int h, int cardSlot, int cardType)
{
    DrawRectangle(x, y, w, h, (Color){ 20, 28, 44, 235 });
    DrawRectangleLines(x, y, w, h, (Color){ 250, 212, 102, 220 });
    DrawText("Pontos da Carta", x + 10, y + 8, 18, YELLOW);
    DrawText(TextFormat("Slot %d  %s", cardSlot + 1, CardTypeName(cardType)), x + 10, y + 30, 14, RAYWHITE);

    int rowY = y + 54;
    for (int row = 0; row < 3; row++)
    {
        int leftIndex = row * 2;
        int rightIndex = leftIndex + 1;
        int leftValue = CardBonusForPortalCard(cardSlot, cardType, CardBonusElementToMonsterElement(leftIndex));
        int rightValue = CardBonusForPortalCard(cardSlot, cardType, CardBonusElementToMonsterElement(rightIndex));

        DrawText(TextFormat("%s +%d", CardBonusElementLabel(leftIndex), leftValue), x + 10, rowY + row * 22, 13, SKYBLUE);
        DrawText(TextFormat("%s +%d", CardBonusElementLabel(rightIndex), rightValue), x + (w / 2) + 2, rowY + row * 22, 13, SKYBLUE);
    }
}

void InicializarEstadoPreparacaoDeck(EstadoPreparacaoDeck *deckSetup)
{
    if (!deckSetup) return;

    for (int p = 0; p < 2; p++)
    {
        for (int i = 0; i < 3; i++)
        {
            deckSetup->cardTypes[p][i] = 0;
            deckSetup->monsterTypes[p][i] = 0;
            deckSetup->monsterElements[p][i] = 0;
        }
    }
}

void DesenharTelaMenuPrincipal(int screenWidth, int screenHeight)
{
    DrawText(
        "BAKUGAN PARK",
        screenWidth / 2 - MeasureText("BAKUGAN PARK", 50) / 2,
        150,
        50,
        BLACK
    );

    DrawText(
        "PRESSIONE ENTER PARA INICIAR",
        screenWidth / 2 - MeasureText("PRESSIONE ENTER PARA INICIAR", 24) / 2,
        250,
        24,
        DARKGRAY
    );

    DrawText(
        "ESC para sair",
        screenWidth / 2 - MeasureText("ESC para sair", 20) / 2,
        300,
        20,
        GRAY
    );
}

void DesenharTelaVitoria(int screenWidth, int screenHeight, int jogadorVencedor, int player1Score, int player2Score)
{
    DrawText(
        TextFormat("JOGADOR %d VENCEU!", jogadorVencedor + 1),
        screenWidth / 2 - MeasureText(TextFormat("JOGADOR %d VENCEU!", jogadorVencedor + 1), 45) / 2,
        130,
        45,
        BLACK
    );

    DrawText(
        TextFormat("PLACAR: P1 %d x %d P2", player1Score, player2Score),
        screenWidth / 2 - MeasureText(TextFormat("PLACAR: P1 %d x %d P2", player1Score, player2Score), 25) / 2,
        210,
        25,
        DARKGRAY
    );

    DrawText(
        "S - estatisticas",
        screenWidth / 2 - MeasureText("S - estatisticas", 22) / 2,
        290,
        22,
        GRAY
    );

    DrawText(
        "ENTER ou H - voltar ao menu",
        screenWidth / 2 - MeasureText("ENTER ou H - voltar ao menu", 22) / 2,
        330,
        22,
        GRAY
    );
}

void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3])
{
    int barH = 112;
    int y = screenHeight - barH;
    Color turnColor = GetTurnColor(activePlayer);

    DrawRectangle(0, y, screenWidth, barH, (Color){210, 210, 210, 255});
    DrawRectangle(0, y, screenWidth, 4, turnColor);

    DrawText(activePlayer == 0 ? "Vez: P1" : "Vez: P2", screenWidth / 2 - 35, y + 6, 18, turnColor);

    int padding = 20;
    int iconSize = 44;

    int mx = padding;

    DrawText(activePlayer == 0 ? "Monstros P1:" : "Monstros P2:", mx, y + 8, 18, turnColor);

    for (int i = 0; i < 3; i++)
    {
        int cx = mx + 120 + i * (iconSize + 10);
        int cy = y + 24 + iconSize / 2;
        Color col = playerMonsters[activePlayer][i] ? turnColor : GRAY;

        DrawCircle(cx, cy, iconSize / 2, col);
        DrawCircleLines(cx, cy, iconSize / 2, BLACK);
        DrawText(TextFormat("%d", i + 1), cx - 4, cy - 8, 12, WHITE);
    }

    int cxBase = screenWidth - padding - (iconSize + 10) * 3;

    DrawText(activePlayer == 0 ? "Cartas P1:" : "Cartas P2:", cxBase - 92, y + 8, 18, turnColor);

    for (int i = 0; i < 3; i++)
    {
        int rx = cxBase + i * (iconSize + 10);
        int ry = y + 18;

        DrawRectangleLines(rx, ry, iconSize, iconSize, turnColor);

        if (playerCards[activePlayer][i])
            DrawRectangle(rx + 2, ry + 2, iconSize - 4, iconSize - 4, ORANGE);
        else
            DrawRectangle(rx + 2, ry + 2, iconSize - 4, iconSize - 4, LIGHTGRAY);

        DrawText(TextFormat("%d", i + 1), rx + 16, ry + 14, 14, BLACK);
    }
}

void DrawChoicePanel(int screenWidth, int screenHeight, int activePlayer, bool canPickMonster, bool selectingSlot, bool selectingMonster, const int availableSlots[3], int selectedSlot, const int cardTypes[3], const int monsterTypes[3], const int monsterElements[3])
{
    int panelW = 560;
    int panelH = selectingSlot ? 270 : 220;
    int x = 24;
    int y = selectingSlot ? (screenHeight - panelH - 120) : 116;
    Color turnColor = GetTurnColor(activePlayer);

    DrawRectangleRounded((Rectangle){ (float)x, (float)y, (float)panelW, (float)panelH }, 0.12f, 10, (Color){ 16, 22, 34, 238 });
    DrawRectangleRoundedLines((Rectangle){ (float)x, (float)y, (float)panelW, (float)panelH }, 0.12f, 10, turnColor);
    DrawRectangle(x, y, panelW, 6, turnColor);

    DrawRectangle(x + 18, y + 18, 52, 52, Fade(turnColor, 0.95f));
    DrawRectangleLines(x + 18, y + 18, 52, 52, WHITE);
    DrawText(activePlayer == 0 ? "P1" : "P2", x + 30, y + 31, 20, WHITE);
    DrawText(selectingSlot ? "Escolha o slot" : "Escolha", x + 84, y + 22, 24, RAYWHITE);

    if (!selectingSlot)
    {
        Rectangle cardBox = { (float)(x + 18), (float)(y + 86), 250.0f, 96.0f };
        Rectangle monsterBox = { (float)(x + 292), (float)(y + 86), 250.0f, 96.0f };

        DrawRectangleRounded(cardBox, 0.16f, 10, Fade((Color){ 225, 120, 95, 255 }, 0.18f));
        DrawRectangleRoundedLines(cardBox, 0.16f, 10, (Color){ 255, 170, 150, 255 });
        DrawRectangleRounded(monsterBox, 0.16f, 10, canPickMonster ? Fade((Color){ 100, 170, 140, 255 }, 0.20f) : Fade((Color){ 100, 100, 100, 255 }, 0.18f));
        DrawRectangleRoundedLines(monsterBox, 0.16f, 10, canPickMonster ? (Color){ 160, 240, 200, 255 } : (Color){ 150, 150, 150, 255 });

        DrawText("Carta", x + 36, y + 102, 24, RAYWHITE);
        DrawText("Bakugan", x + 310, y + 102, 24, RAYWHITE);
        DrawCircle(x + 220, y + 114, 10, ORANGE);
        DrawCircle(x + 492, y + 114, 10, canPickMonster ? GREEN : GRAY);

        DrawText("C", x + 42, y + 136, 18, BLACK);
        DrawText("M", x + 316, y + 136, 18, BLACK);

        DrawText("ENTER", x + 18, y + 198, 14, (Color){ 210, 210, 210, 255 });
        DrawText(canPickMonster ? "pronto" : "sem carta", x + 420, y + 198, 14, canPickMonster ? (Color){ 170, 255, 190, 255 } : (Color){ 255, 160, 160, 255 });
    }
    else
    {
        int slotX = x + 18;
        int slotY = y + 86;
        int slotW = 166;
        int slotH = 92;

        for (int i = 0; i < 3; i++)
        {
            bool selected = (i == selectedSlot);
            bool available = availableSlots[i] != 0;
            int sx = slotX + i * (slotW + 12);
            Rectangle box = { (float)sx, (float)slotY, (float)slotW, (float)slotH };
            Color back = selectingMonster ? (available ? Fade((Color){ 100, 170, 140, 255 }, 0.22f) : (Color){ 72, 72, 72, 255 }) : (available ? Fade(CardTypeColor(cardTypes[i]), 0.22f) : (Color){ 72, 72, 72, 255 });

            DrawRectangleRounded(box, 0.16f, 10, back);
            DrawRectangleRoundedLines(box, 0.16f, 10, selected ? WHITE : (Color){ 255, 255, 255, 90 });

            DrawText(TextFormat("%d", i + 1), sx + 12, slotY + 10, 18, BLACK);
            if (available)
            {
                Texture2D preview = selectingMonster ? ObterBakuganTexture(monsterTypes[i]) : ObterCartaTexturePorSlot(i, cardTypes[i]);
                DrawTextureEx(preview, (Vector2){ (float)sx + 54.0f, (float)slotY + 14.0f }, 0.0f, 0.30f, WHITE);
                DrawText(selectingMonster ? BakuganTypeName(monsterTypes[i]) : CardTypeName(cardTypes[i]), sx + 14, slotY + 54, 15, RAYWHITE);
                DrawText(selectingMonster ? BakuganElementName(monsterElements[i]) : "ok", sx + 14, slotY + 70, 12, selectingMonster ? SKYBLUE : (Color){ 170, 255, 190, 255 });
            }
            else
            {
                DrawRectangle(sx + 56, slotY + 18, 42, 42, (Color){ 120, 120, 120, 255 });
                DrawText("x", sx + 72, slotY + 24, 20, WHITE);
                DrawText("usado", sx + 14, slotY + 54, 16, (Color){ 220, 220, 220, 255 });
            }
        }

        DrawRectangle(x + 18, y + 194, panelW - 36, 42, (Color){ 230, 235, 244, 230 });
        DrawRectangleLines(x + 18, y + 194, panelW - 36, 42, (Color){ 140, 140, 150, 255 });
        DrawText("A/D ou 1-3 | ENTER", x + 26, y + 206, 16, DARKGRAY);
    }
}

void DrawScoreBoard(int screenWidth, int hudY, int player1Score, int player2Score, Texture2D iconP1, Texture2D iconP2)
{
    int panelW = 248;
    int panelH = 76;
    int leftX = 14;
    int rightX = screenWidth - 14 - panelW;

    DrawRectangle(leftX, hudY, panelW, panelH, (Color){ 18, 24, 36, 230 });
    DrawRectangleLines(leftX, hudY, panelW, panelH, BLUE);
    DrawTexture(iconP1, leftX + 10, hudY + 14, WHITE);
    DrawText("P1", leftX + 72, hudY + 10, 20, SKYBLUE);
    DrawText(TextFormat("%d/3", player1Score), leftX + 114, hudY + 24, 34, WHITE);

    for (int i = 0; i < 3; i++)
    {
        int bx = leftX + 66 + i * 32;
        DrawRectangleLines(bx, hudY + 52, 24, 18, RAYWHITE);
        if (i < player1Score)
            DrawRectangle(bx + 3, hudY + 55, 18, 12, GOLD);
    }

    DrawRectangle(rightX, hudY, panelW, panelH, (Color){ 18, 24, 36, 230 });
    DrawRectangleLines(rightX, hudY, panelW, panelH, RED);
    DrawTexture(iconP2, rightX + 10, hudY + 14, WHITE);
    DrawText("P2", rightX + 72, hudY + 10, 20, (Color){ 255, 150, 150, 255 });
    DrawText(TextFormat("%d/3", player2Score), rightX + 114, hudY + 24, 34, WHITE);

    for (int i = 0; i < 3; i++)
    {
        int bx = rightX + 66 + i * 32;
        DrawRectangleLines(bx, hudY + 52, 24, 18, RAYWHITE);
        if (i < player2Score)
            DrawRectangle(bx + 3, hudY + 55, 18, 12, GOLD);
    }
}

void DrawBattleActivationPrompt(int screenWidth, int screenHeight, bool p0Present, bool p1Present, bool p0Activated, bool p1Activated)
{
    if (!p0Present && !p1Present) return;

    int cy = screenHeight / 2 - 58;
    int cx = screenWidth / 2;

    DrawRectangleRounded((Rectangle){ (float)(cx - 250), (float)(cy - 62), 500.0f, 150.0f }, 0.18f, 10, (Color){ 18, 24, 36, 235 });
    DrawRectangleRoundedLines((Rectangle){ (float)(cx - 250), (float)(cy - 62), 500.0f, 150.0f }, 0.18f, 10, (Color){ 255, 255, 255, 70 });
    DrawText("Ative a carta para iniciar a batalha", cx - MeasureText("Ative a carta para iniciar a batalha", 18) / 2, cy - 40, 18, RAYWHITE);

    if (p0Present)
    {
        Rectangle card = { (float)(cx - 225), (float)(cy - 2), 200.0f, 70.0f };
        DrawRectangleRounded(card, 0.22f, 10, p0Activated ? (Color){ 255, 166, 90, 255 } : (Color){ 45, 55, 78, 255 });
        DrawRectangleRoundedLines(card, 0.22f, 10, p0Activated ? ORANGE : (Color){ 255, 255, 255, 65 });
        DrawText("P1", cx - 205, cy + 10, 20, RAYWHITE);
        DrawText(p0Activated ? "Ativado" : "Pronto", cx - 205, cy + 34, 14, p0Activated ? (Color){ 255, 240, 220, 255 } : SKYBLUE);
        DrawRectangleRounded((Rectangle){ (float)(cx - 98), (float)(cy + 10), 42.0f, 42.0f }, 0.35f, 10, p0Activated ? (Color){ 255, 215, 140, 255 } : ORANGE);
        DrawRectangleRoundedLines((Rectangle){ (float)(cx - 98), (float)(cy + 10), 42.0f, 42.0f }, 0.35f, 10, WHITE);
        DrawText("F", cx - 85, cy + 18, 24, BLACK);
    }

    if (p1Present)
    {
        Rectangle card = { (float)(cx + 25), (float)(cy - 2), 200.0f, 70.0f };
        DrawRectangleRounded(card, 0.22f, 10, p1Activated ? (Color){ 255, 120, 120, 255 } : (Color){ 45, 55, 78, 255 });
        DrawRectangleRoundedLines(card, 0.22f, 10, p1Activated ? RED : (Color){ 255, 255, 255, 65 });
        DrawText("P2", cx + 45, cy + 10, 20, RAYWHITE);
        DrawText(p1Activated ? "Ativado" : "Pronto", cx + 45, cy + 34, 14, p1Activated ? (Color){ 255, 240, 240, 255 } : SKYBLUE);
        DrawRectangleRounded((Rectangle){ (float)(cx + 152), (float)(cy + 10), 42.0f, 42.0f }, 0.35f, 10, p1Activated ? (Color){ 255, 190, 190, 255 } : RED);
        DrawRectangleRoundedLines((Rectangle){ (float)(cx + 152), (float)(cy + 10), 42.0f, 42.0f }, 0.35f, 10, WHITE);
        DrawText("L", cx + 166, cy + 18, 24, BLACK);
    }
}

void DrawBattleCardPreview(int screenWidth, int screenHeight, int owner, int cardType, int slot, Texture2D cardTexture, bool opening)
{
    int panelW = 280;
    int panelH = 58;
    int x = (screenWidth - panelW) / 2;
    int y = opening ? 152 : 160;

    DrawRectangle(x, y, panelW, panelH, (Color){ 15, 20, 32, 235 });
    DrawRectangleLines(x, y, panelW, panelH, owner == 0 ? ORANGE : RED);
    DrawText(opening ? "CARTA PORTAL" : "CARTA EM JOGO", x + 12, y + 8, 14, YELLOW);
    DrawText(TextFormat("P%d  SLOT %d", owner + 1, slot + 1), x + 12, y + 26, 12, RAYWHITE);
    DrawText(CardTypeName(cardType), x + 12, y + 40, 14, WHITE);
    DrawTextureEx(cardTexture, (Vector2){ (float)x + 208.0f, (float)y + 8.0f }, 0.0f, 0.28f, WHITE);
}

static float LerpFloatLocal(float a, float b, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return a + (b - a) * t;
}

static const float UI_PI = 3.14159265f;

void DrawMinimalBattleUI(int screenWidth, int screenHeight, int tileGX, int tileGZ, bool portalOpening, bool awaitingActivation, bool statusApplied, int battlePortalTimer)
{
    static int animElapsed[2] = {0,0};
    static int animDur = 60;
    static int animTarget[2] = {0,0};
    static bool animActive[2] = {false,false};
    static bool animStartedOnce[2] = { false, false };

    TileEntity te = ObterTileEm(tileGX, tileGZ);

    MonsterPlacement mP1 = EmptyMonsterPlacement();
    MonsterPlacement mP2 = EmptyMonsterPlacement();
    for (int i = 0; i < te.monsterCount; i++) {
        if (te.monsters[i].owner == 0) mP1 = te.monsters[i];
        if (te.monsters[i].owner == 1) mP2 = te.monsters[i];
    }

    int barX = 24;
    int barY = 18;
    int barW = screenWidth - 48;
    int barH = 124;
    int sideW = 240;
    int sideH = 76;
    int cardW = 240;
    int cardH = 76;
    int leftX = barX + 16;
    int rightX = screenWidth - barX - 16 - sideW;
    int centerX = (screenWidth - cardW) / 2;
    int y = barY + 34;

    // reinicia o estado quando um novo portal começa a abrir
    if (portalOpening && !statusApplied)
    {
        animElapsed[0] = animElapsed[1] = 0;
        animTarget[0] = animTarget[1] = 0;
        animActive[0] = animActive[1] = false;
        animStartedOnce[0] = animStartedOnce[1] = false;
    }

    // iniciar animação apenas quando o status da carta foi aplicado (uma vez)
    if (statusApplied)
    {
        if (mP1.owner != -1 && !animStartedOnce[0]) { animActive[0] = true; animElapsed[0] = 0; animTarget[0] = mP1.power; animStartedOnce[0] = true; }
        if (mP2.owner != -1 && !animStartedOnce[1]) { animActive[1] = true; animElapsed[1] = 0; animTarget[1] = mP2.power; animStartedOnce[1] = true; }
    }

    // atualizar contadores
    for (int i = 0; i < 2; i++) if (animActive[i]) { animElapsed[i]++; if (animElapsed[i] >= animDur) animActive[i] = false; }

    DrawRectangleRounded((Rectangle){ (float)barX, (float)barY, (float)barW, (float)barH }, 0.10f, 10, (Color){ 14, 20, 32, 240 });
    DrawRectangleLinesEx((Rectangle){ (float)barX, (float)barY, (float)barW, (float)barH }, 2.0f, (Color){ 250, 212, 102, 220 });
    DrawText("BATTLE", barX + 18, barY + 12, 18, YELLOW);
    DrawText(portalOpening ? "PORTAL ABRINDO" : (awaitingActivation ? "PORTAL ATIVO" : "DUEL MODE"), barX + 120, barY + 12, 16, RAYWHITE);

    DrawRectangleRounded((Rectangle){ (float)leftX, (float)y, (float)sideW, (float)sideH }, 0.12f, 8, Fade(ORANGE, 0.92f));
    DrawRectangleLinesEx((Rectangle){ (float)leftX, (float)y, (float)sideW, (float)sideH }, 2.0f, BLACK);
    DrawCircle(leftX + 34, y + sideH / 2, 24, (Color){ 245, 180, 90, 255 });
    if (mP1.owner != -1) {
        Texture2D t = ObterBakuganTexture(mP1.type);
        DrawTextureEx(t, (Vector2){ (float)leftX + 11.0f, (float)y + 10.0f }, 0.0f, 0.42f, WHITE);
    }
    DrawText("P1", leftX + 70, y + 9, 16, BLACK);
    DrawText("G POWER", leftX + 70, y + 26, 12, BLACK);
    int displayP1 = mP1.owner != -1 ? (animActive[0] ? (int)LerpFloatLocal(0, (float)animTarget[0], (float)animElapsed[0]/(float)animDur) : mP1.power) : 0;
    DrawText(TextFormat("%d", displayP1), leftX + 122, y + 21, animActive[0] ? 26 : 22, GOLD);

    DrawRectangleRounded((Rectangle){ (float)rightX, (float)y, (float)sideW, (float)sideH }, 0.12f, 8, Fade(RED, 0.92f));
    DrawRectangleLinesEx((Rectangle){ (float)rightX, (float)y, (float)sideW, (float)sideH }, 2.0f, BLACK);
    DrawCircle(rightX + 34, y + sideH / 2, 24, (Color){ 240, 120, 120, 255 });
    if (mP2.owner != -1) {
        Texture2D t2 = ObterBakuganTexture(mP2.type);
        DrawTextureEx(t2, (Vector2){ (float)rightX + 11.0f, (float)y + 10.0f }, 0.0f, 0.42f, WHITE);
    }
    DrawText("P2", rightX + 70, y + 9, 16, BLACK);
    DrawText("G POWER", rightX + 70, y + 26, 12, BLACK);
    int displayP2 = mP2.owner != -1 ? (animActive[1] ? (int)LerpFloatLocal(0, (float)animTarget[1], (float)animElapsed[1]/(float)animDur) : mP2.power) : 0;
    DrawText(TextFormat("%d", displayP2), rightX + 122, y + 21, animActive[1] ? 26 : 22, GOLD);

    DrawRectangleRounded((Rectangle){ (float)centerX, (float)y - 4, (float)cardW, (float)cardH }, 0.12f, 8, (Color){ 26, 34, 50, 240 });
    DrawRectangleLinesEx((Rectangle){ (float)centerX, (float)y - 4, (float)cardW, (float)cardH }, 2.0f, (Color){ 255, 221, 117, 220 });
    DrawText("CARTA PORTAL", centerX + 14, y + 4, 12, YELLOW);
    DrawText(CardTypeName(te.card.type), centerX + 14, y + 24, 18, RAYWHITE);
    DrawText(TextFormat("P%d | SLOT %d", te.card.owner + 1, te.card.slot + 1), centerX + 14, y + 44, 12, SKYBLUE);
    Texture2D cardTex = ObterCartaTexturePorSlot(te.card.slot, te.card.type);
    DrawTextureEx(cardTex, (Vector2){ (float)centerX + 160.0f, (float)y + 10.0f }, 0.0f, 0.32f, WHITE);
}

void DrawGameHints(int screenWidth, int screenHeight, const char *placeMessage)
{
    DrawText("ENTER marca tile | C carta | M monstro", 10, screenHeight - 52, 14, DARKGRAY);

    if (placeMessage && placeMessage[0] != '\0')
    {
        DrawText(placeMessage, 10, screenHeight - 72, 14, RED);
    }
}

void DrawBattleFeedbackOverlay(int screenWidth, int screenHeight, const char *battleMessage, int battleResolveTimer, int placedFeedbackTimer)
{
    if (battleResolveTimer > 0 && battleMessage && battleMessage[0] != '\0')
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 35});

        DrawText(
            battleMessage,
            screenWidth / 2 - MeasureText(battleMessage, 20) / 2,
            40,
            20,
            YELLOW
        );
    }

    if (placedFeedbackTimer > 0)
    {
        float alpha = placedFeedbackTimer / 60.0f;
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, (unsigned char)(50 * alpha)});
    }
}

void DrawPlacementPrecisionBar(int screenWidth, int screenHeight, int chosenSide, float cursorT, int activePlayer)
{
    int barW = 420;
    int barH = 24;
    int x = (screenWidth - barW) / 2;
    int y = screenHeight - 170;

    DrawRectangle(x, y, barW, barH, (Color){35, 35, 35, 230});
    DrawRectangle(x, y, barW / 2, barH, (Color){225, 135, 40, 255});
    DrawRectangle(x + barW / 2, y, barW / 2, barH, (Color){225, 60, 60, 255});
    DrawRectangleLines(x, y, barW, barH, BLACK);
    DrawLine(x + barW / 2, y, x + barW / 2, y + barH, BLACK);

    int cursorX = x + (int)(cursorT * (float)barW);

    DrawRectangle(cursorX - 4, y - 6, 8, barH + 12, WHITE);

    const char *sideLabel = (chosenSide == MONSTER_SIDE_LEFT) ? "Lado escolhido: esquerda" : "Lado escolhido: direita";

    DrawText(sideLabel, x, y - 28, 18, activePlayer == 0 ? ORANGE : RED);
    DrawText("A/D muda lado | ESPACO confirma", x, y + 34, 16, DARKGRAY);
}