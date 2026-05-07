#ifndef UI_H
#define UI_H

#include "raylib.h"

// Draw bottom menu with monster slots (circles) and card slots (rectangles)
void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3]);

// Draw menu when tile is marked: user chooses carta (C) or monstro (M)
// canPickMonster: true if player already has a card on map (can pick monster)
void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer);

#endif // UI_H
