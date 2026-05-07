#ifndef UI_H
#define UI_H

#include "raylib.h"

// Desenha o menu inferior com slots de monstro (círculos) e cartas (retângulos)
void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3]);

// Desenha o menu quando o tile está marcado: o jogador escolhe carta (C) ou monstro (M)
// canPickMonster: verdadeiro se o jogador já tem uma carta no mapa (pode escolher monstro)
void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer);

#endif // fim de UI_H
