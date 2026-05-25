#include "ui.h"

static Color GetTurnColor(int activePlayer)
{
    return (activePlayer == 0) ? BLUE : RED;
}

void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3])
{
    int barH = 112;
    int y = screenHeight - barH;
    Color turnColor = GetTurnColor(activePlayer);
    DrawRectangle(0, y, screenWidth, barH, (Color){210,210,210,255});
    DrawRectangle(0, y, screenWidth, 4, turnColor);

    DrawText(activePlayer == 0 ? "Vez: P1" : "Vez: P2", screenWidth / 2 - 35, y + 6, 18, turnColor);

    int padding = 20;
    int iconSize = 44;

    // Monstros (lado esquerdo)
    int mx = padding;
    DrawText(activePlayer == 0 ? "Monstros P1:" : "Monstros P2:", mx, y + 8, 18, turnColor);
    for (int i = 0; i < 3; i++) {
        int cx = mx + 120 + i * (iconSize + 10);
        int cy = y + 24 + iconSize/2;
        Color col = playerMonsters[activePlayer][i] ? turnColor : GRAY;
        DrawCircle(cx, cy, iconSize/2, col);
        DrawCircleLines(cx, cy, iconSize/2, BLACK);
        DrawText(TextFormat("%d", i + 1), cx - 4, cy - 8, 12, WHITE);
    }

    // Cartas (lado direito)
    int cxBase = screenWidth - padding - (iconSize+10)*3;
    DrawText(activePlayer == 0 ? "Cartas P1:" : "Cartas P2:", cxBase - 92, y + 8, 18, turnColor);
    for (int i = 0; i < 3; i++) {
        int rx = cxBase + i * (iconSize + 10);
        int ry = y + 18;
        DrawRectangleLines(rx, ry, iconSize, iconSize, turnColor);
        if (playerCards[activePlayer][i]) DrawRectangle(rx+2, ry+2, iconSize-4, iconSize-4, ORANGE);
        else DrawRectangle(rx+2, ry+2, iconSize-4, iconSize-4, LIGHTGRAY);
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

    DrawRectangle(x, y, menuW, menuH, (Color){230,230,230,255});
    DrawRectangleLines(x, y, menuW, menuH, turnColor);
    DrawRectangle(x, y, menuW, 4, turnColor);

    DrawText(activePlayer == 0 ? "P1 escolhe" : "P2 escolhe", x + 18, y + 10, 20, turnColor);

    int cartaY = y + 42;
    DrawRectangle(x + 20, cartaY, 90, 60, ORANGE);
    DrawRectangleLines(x + 20, cartaY, 90, 60, BLACK);
    DrawText("Carta", x + 38, cartaY + 18, 16, BLACK);
    DrawText("(C)", x + 44, cartaY + 37, 12, DARKGRAY);

    int monstroY = y + 42;
    int monstrox = x + 130;
    Color monstroCol = canPickMonster ? turnColor : DARKGRAY;
    DrawRectangle(monstrox, monstroY, 90, 60, monstroCol);
    DrawRectangleLines(monstrox, monstroY, 90, 60, canPickMonster ? BLACK : GRAY);
    DrawText("Monstro", monstrox + 10, monstroY + 18, 16, canPickMonster ? WHITE : GRAY);
    DrawText("(M)", monstrox + 35, monstroY + 37, 12, canPickMonster ? WHITE : GRAY);

    if (!canPickMonster) {
        DrawText("Precisa de carta no mapa primeiro", x + 18, y + 118, 12, RED);
    }
}

void DrawScoreBoard(int screenWidth, int hudY, int player1Score, int player2Score, Texture2D iconP1, Texture2D iconP2)
{
    DrawTexture(iconP1, 10, hudY, WHITE);
    DrawText("P1", 64, hudY + 6, 18, BLACK);

    for (int i = 0; i < 3; i++) {
        int bx = 104 + i * 22;
        DrawRectangleLines(bx, hudY + 6, 18, 18, BLACK);
        if (i < player1Score) {
            DrawRectangle(bx + 1, hudY + 7, 16, 16, GOLD);
        }
    }

    int p2x = screenWidth - 58;
    DrawTexture(iconP2, p2x, hudY, WHITE);
    DrawText("P2", p2x - 36, hudY + 6, 18, BLACK);

    for (int i = 0; i < 3; i++) {
        int bx = p2x - 120 + i * 22;
        DrawRectangleLines(bx, hudY + 6, 18, 18, BLACK);
        if (i < player2Score) {
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

    if (p0Present) {
        const char *label = p0Activated ? "P1 ativado (F)" : "P1: pressione F";
        DrawText(label, cx - 160, cy, 16, p0Activated ? ORANGE : DARKGRAY);
    }

    if (p1Present) {
        const char *label = p1Activated ? "P2 ativado (L)" : "P2: pressione L";
        DrawText(label, cx + 20, cy, 16, p1Activated ? RED : DARKGRAY);
    }
}

void DrawGameHints(int screenWidth, int screenHeight, const char *placeMessage)
{
    DrawText("ENTER marca tile | C carta | M monstro", 10, screenHeight - 52, 14, DARKGRAY);

    if (placeMessage && placeMessage[0] != '\0') {
        DrawText(placeMessage, 10, screenHeight - 72, 14, RED);
    }
}

void DrawBattleFeedbackOverlay(int screenWidth, int screenHeight, const char *battleMessage, int battleResolveTimer, int placedFeedbackTimer)
{
    if (battleResolveTimer > 0 && battleMessage && battleMessage[0] != '\0') {
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 35});
        DrawText(
            battleMessage,
            screenWidth / 2 - MeasureText(battleMessage, 20) / 2,
            40,
            20,
            YELLOW
        );
    }

    if (placedFeedbackTimer > 0) {
        float alpha = placedFeedbackTimer / 60.0f;
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, (unsigned char)(50 * alpha)});
    }
}
