#include "ui.h"
#include "menu.h"

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
    DrawText(available ? "Disponivel" : "Usado", x + 76, y + 56, 13, available ? DARKGREEN : DARKGRAY);
}

static Color GetTurnColor(int activePlayer)
{
    return (activePlayer == 0) ? BLUE : RED;
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

void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer, const int cardTypes[3], const int monsterTypes[3], const int monsterElements[3])
{
    int menuW = 520;
    int menuH = 214;
    int x = (screenWidth - menuW) / 2;
    int y = (screenHeight - menuH) / 2;
    Color turnColor = GetTurnColor(activePlayer);

    DrawRectangle(x, y, menuW, menuH, (Color){230, 230, 230, 255});
    DrawRectangleLines(x, y, menuW, menuH, turnColor);
    DrawRectangle(x, y, menuW, 4, turnColor);

    DrawText(activePlayer == 0 ? "P1 escolhe" : "P2 escolhe", x + 18, y + 10, 20, turnColor);
    DrawText("Escolha visualmente o que quer jogar", x + 18, y + 34, 14, DARKGRAY);

    DrawRectangle(x + 18, y + 62, 212, 112, Fade(CardTypeColor(cardTypes[0]), 0.18f));
    DrawRectangleLines(x + 18, y + 62, 212, 112, turnColor);
    DrawText("CARTA", x + 28, y + 70, 18, BLACK);
    DrawText("Pressione C", x + 130, y + 70, 14, DARKGRAY);
    for (int i = 0; i < 3; i++)
    {
        DrawMiniCardPreview(x + 28 + i * 64, y + 98, 54, 60, cardTypes[i], false, true);
    }

    DrawRectangle(x + 264, y + 62, 238, 112, canPickMonster ? Fade(turnColor, 0.18f) : (Color){ 185, 185, 185, 255 });
    DrawRectangleLines(x + 264, y + 62, 238, 112, canPickMonster ? turnColor : GRAY);
    DrawText("BAKUGAN", x + 274, y + 70, 18, BLACK);
    DrawText("Pressione M", x + 390, y + 70, 14, DARKGRAY);
    for (int i = 0; i < 3; i++)
    {
        DrawMiniMonsterPreview(x + 274 + i * 72, y + 98, 60, 60, monsterTypes[i], monsterElements[i], false, canPickMonster);
    }

    if (!canPickMonster)
        DrawText("Precisa de carta no mapa primeiro", x + 18, y + 186, 12, RED);
}

void DrawScoreBoard(int screenWidth, int hudY, int player1Score, int player2Score, Texture2D iconP1, Texture2D iconP2)
{
    DrawTexture(iconP1, 10, hudY, WHITE);
    DrawText("P1", 64, hudY + 6, 18, BLACK);

    for (int i = 0; i < 3; i++)
    {
        int bx = 104 + i * 22;

        DrawRectangleLines(bx, hudY + 6, 18, 18, BLACK);

        if (i < player1Score)
        {
            DrawRectangle(bx + 1, hudY + 7, 16, 16, GOLD);
        }
    }

    int p2x = screenWidth - 58;

    DrawTexture(iconP2, p2x, hudY, WHITE);
    DrawText("P2", p2x - 36, hudY + 6, 18, BLACK);

    for (int i = 0; i < 3; i++)
    {
        int bx = p2x - 120 + i * 22;

        DrawRectangleLines(bx, hudY + 6, 18, 18, BLACK);

        if (i < player2Score)
        {
            DrawRectangle(bx + 1, hudY + 7, 16, 16, GOLD);
        }
    }
}

void DrawBattleActivationPrompt(int screenWidth, int screenHeight, bool p0Present, bool p1Present, bool p0Activated, bool p1Activated)
{
    if (!p0Present && !p1Present) return;

    int cy = screenHeight / 2 - 40;
    int cx = screenWidth / 2;

    DrawText("Ativar carta para iniciar batalha:", cx - 180, cy - 30, 18, BLACK);

    if (p0Present)
    {
        const char *label = p0Activated ? "P1 ativado (F)" : "P1: pressione F";
        DrawText(label, cx - 160, cy, 16, p0Activated ? ORANGE : DARKGRAY);
    }

    if (p1Present)
    {
        const char *label = p1Activated ? "P2 ativado (L)" : "P2: pressione L";
        DrawText(label, cx + 20, cy, 16, p1Activated ? RED : DARKGRAY);
    }
}

void DrawPlacementSlotMenu(int screenWidth, int screenHeight, int activePlayer, const int availableSlots[3], int selectedSlot, bool selectingMonster, const int cardTypes[3], const int monsterTypes[3], const int monsterElements[3])
{
    int menuW = 640;
    int menuH = 214;
    int x = (screenWidth - menuW) / 2;
    int y = (screenHeight - menuH) / 2;
    Color turnColor = GetTurnColor(activePlayer);

    const char *title = selectingMonster ? "Escolha o Bakugan" : "Escolha a Carta";
    const char *subtitle = selectingMonster ? "1-3 escolhe o bakugan | ENTER confirma | ESC cancela" : "1-3 escolhe a carta | ENTER confirma | ESC cancela";

    DrawRectangle(x, y, menuW, menuH, (Color){230, 230, 230, 255});
    DrawRectangleLines(x, y, menuW, menuH, turnColor);
    DrawRectangle(x, y, menuW, 4, turnColor);

    DrawText(activePlayer == 0 ? "P1" : "P2", x + 18, y + 10, 20, turnColor);
    DrawText(title, x + 60, y + 10, 20, BLACK);
    DrawText(subtitle, x + 18, y + 34, 14, DARKGRAY);

    int slotX = x + 18;
    int slotY = y + 68;
    int slotW = 168;
    int slotH = 108;

    for (int i = 0; i < 3; i++)
    {
        bool selected = (i == selectedSlot);
        bool available = availableSlots[i] != 0;
        int sx = slotX + i * (slotW + 12);
        int sy = slotY;
        Rectangle box = { (float)sx, (float)sy, (float)slotW, (float)slotH };
        Color back = selectingMonster ? Fade(turnColor, available ? 0.18f : 0.06f) : (available ? Fade(CardTypeColor(cardTypes[i]), 0.20f) : LIGHTGRAY);

        DrawRectangleRec(box, back);
        DrawRectangleLinesEx(box, selected ? 3.0f : 2.0f, selected ? BLACK : DARKGRAY);
        DrawText(TextFormat("%d", i + 1), sx + 10, sy + 8, 18, BLACK);

        if (selectingMonster)
        {
            DrawMiniMonsterPreview(sx + 10, sy + 28, 148, 70, monsterTypes[i], monsterElements[i], selected, available);
        }
        else
        {
            DrawMiniCardPreview(sx + 10, sy + 28, 148, 70, cardTypes[i], selected, available);
        }

        DrawText(available ? "Disponivel" : "Usado", sx + 10, sy + 86, 12, available ? DARKGREEN : DARKGRAY);
    }

    int previewX = x + 540;
    int previewY = y + 18;
    int previewType = selectingMonster ? monsterTypes[selectedSlot < 0 ? 0 : selectedSlot] : cardTypes[selectedSlot < 0 ? 0 : selectedSlot];
    int previewElement = selectingMonster ? monsterElements[selectedSlot < 0 ? 0 : selectedSlot] : -1;
    Texture2D previewTexture = selectingMonster ? ObterBakuganTexture(previewType) : ObterCartaTexturePorSlot(selectedSlot < 0 ? 0 : selectedSlot, cardTypes[selectedSlot < 0 ? 0 : selectedSlot]);

    DrawRectangle(previewX, previewY, 84, 180, (Color){ 20, 28, 44, 255 });
    DrawRectangleLines(previewX, previewY, 84, 180, turnColor);
    DrawText("ESCOLHIDO", previewX + 10, previewY + 10, 12, YELLOW);

    if (selectingMonster)
    {
        DrawTextureEx(previewTexture, (Vector2){ (float)previewX + 10.0f, (float)previewY + 36.0f }, 0.0f, 0.70f, WHITE);
        DrawText(BakuganTypeName(previewType), previewX - 110, previewY + 126, 14, RAYWHITE);
        DrawText(BakuganElementName(previewElement), previewX - 110, previewY + 146, 14, SKYBLUE);
    }
    else
    {
        DrawTextureEx(previewTexture, (Vector2){ (float)previewX + 8.0f, (float)previewY + 38.0f }, 0.0f, 0.60f, WHITE);
        DrawText(CardTypeName(previewType), previewX - 58, previewY + 136, 14, RAYWHITE);
    }

    DrawText("A/D troca | ENTER confirma", x + 18, y + 182, 14, DARKGRAY);
}

void DrawBattleCardPreview(int screenWidth, int screenHeight, int owner, int cardType, int slot, Texture2D cardTexture, bool opening)
{
    int panelW = 420;
    int panelH = 150;
    int x = (screenWidth - panelW) / 2;
    int y = opening ? 20 : 32;

    DrawRectangle(x, y, panelW, panelH, (Color){ 15, 20, 32, 235 });
    DrawRectangleLines(x, y, panelW, panelH, owner == 0 ? ORANGE : RED);
    DrawText(opening ? "CARTA EM BATALHA" : "CARTA SELECIONADA", x + 18, y + 12, 20, YELLOW);
    DrawText(TextFormat("P%d | Slot %d", owner + 1, slot + 1), x + 18, y + 38, 16, RAYWHITE);
    DrawText(CardTypeName(cardType), x + 18, y + 60, 24, WHITE);
    DrawTextureEx(cardTexture, (Vector2){ (float)x + 280.0f, (float)y + 16.0f }, 0.0f, 0.48f, WHITE);
    DrawText(opening ? "Virando agora" : "Pronta para virar na batalha", x + 18, y + 102, 14, SKYBLUE);
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