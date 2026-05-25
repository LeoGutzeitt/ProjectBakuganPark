#ifndef UI_H
#define UI_H

#include "raylib.h"

// Desenha o menu inferior com slots de monstro (círculos) e cartas (retângulos)
void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3]);

// Desenha o menu quando o tile está marcado: o jogador escolhe carta (C) ou monstro (M)
// canPickMonster: verdadeiro se o jogador já tem uma carta no mapa (pode escolher monstro)
void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer);

void DrawScoreBoard(int screenWidth, int hudY, int player1Score, int player2Score, Texture2D iconP1, Texture2D iconP2);
void DrawBattleActivationPrompt(int screenWidth, int screenHeight, bool p0Present, bool p1Present, bool p0Activated, bool p1Activated);
void DrawGameHints(int screenWidth, int screenHeight, const char *placeMessage);
void DrawBattleFeedbackOverlay(int screenWidth, int screenHeight, const char *battleMessage, int battleResolveTimer, int placedFeedbackTimer);

#endif // fim de UI_H
