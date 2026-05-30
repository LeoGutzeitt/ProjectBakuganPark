#include "ui.h"

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

void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer)
{
    int menuW = 320;
    int menuH = 155;
    int x = (screenWidth - menuW) / 2;
    int y = (screenHeight - menuH) / 2;
    Color turnColor = GetTurnColor(activePlayer);

    DrawRectangle(x, y, menuW, menuH, (Color){230, 230, 230, 255});
    DrawRectangleLines(x, y, menuW, menuH, turnColor);
    DrawRectangle(x, y, menuW, 4, turnColor);

    DrawText(activePlayer == 0 ? "P1 escolhe" : "P2 escolhe", x + 18, y + 10, 20, turnColor);

    int cartaY = y + 42;

    DrawRectangle(x + 20, cartaY, 90, 60, ORANGE);
    DrawRectangleLines(x + 20, cartaY, 90, 60, BLACK);
    DrawText("Carta", x + 38, cartaY + 18, 16, BLACK);
    DrawText("(C)", x + 44, cartaY + 37, 12, DARKGRAY);

    int monstroY = y + 42;
    int monstroX = x + 130;
    Color monstroCol = canPickMonster ? turnColor : DARKGRAY;

    DrawRectangle(monstroX, monstroY, 90, 60, monstroCol);
    DrawRectangleLines(monstroX, monstroY, 90, 60, canPickMonster ? BLACK : GRAY);
    DrawText("Monstro", monstroX + 10, monstroY + 18, 16, canPickMonster ? WHITE : GRAY);
    DrawText("(M)", monstroX + 35, monstroY + 37, 12, canPickMonster ? WHITE : GRAY);

    if (!canPickMonster)
    {
        DrawText("Precisa de carta no mapa primeiro", x + 18, y + 118, 12, RED);
    }
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

void DrawPlacementSlotMenu(int screenWidth, int screenHeight, int activePlayer, const int availableSlots[3], int selectedSlot, bool selectingMonster)
{
    int menuW = 420;
    int menuH = 182;
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

    int slotX = x + 20;
    int slotY = y + 68;
    int slotW = 112;
    int slotH = 72;

    for (int i = 0; i < 3; i++)
    {
        bool selected = (i == selectedSlot);
        bool available = availableSlots[i] != 0;
        Color fill = available ? (selectingMonster ? Fade(turnColor, 0.8f) : ORANGE) : LIGHTGRAY;
        Color border = selected ? BLACK : DARKGRAY;

        DrawRectangle(slotX + i * (slotW + 12), slotY, slotW, slotH, fill);
        DrawRectangleLines(slotX + i * (slotW + 12), slotY, slotW, slotH, border);
        DrawText(TextFormat("%d", i + 1), slotX + i * (slotW + 12) + 12, slotY + 8, 18, BLACK);

        if (available)
            DrawText("Disponivel", slotX + i * (slotW + 12) + 12, slotY + 30, 16, BLACK);
        else
            DrawText("Usado", slotX + i * (slotW + 12) + 24, slotY + 30, 16, DARKGRAY);

        if (selected)
        {
            DrawRectangleLinesEx(
                (Rectangle){
                    (float)(slotX + i * (slotW + 12)),
                    (float)slotY,
                    (float)slotW,
                    (float)slotH
                },
                3.0f,
                BLACK
            );
        }
    }

    DrawText("A/D troca o slot", x + 18, y + 148, 14, DARKGRAY);
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