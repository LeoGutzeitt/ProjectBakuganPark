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

    // Monsters (left side)
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

    // Cards (right side)
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
