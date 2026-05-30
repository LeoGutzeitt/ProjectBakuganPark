#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "monster.h"


typedef enum {
    TELA_MENU_PRINCIPAL,
    TELA_PREPARACAO_DECK,
    TELA_BATALHA,
    TELA_VITORIA,
    TELA_ESTATISTICAS
} TelaAplicacao;

typedef struct {
    int cardTypes[2][3];
    int monsterTypes[2][3];
    int monsterElements[2][3];
} EstadoPreparacaoDeck;

void InicializarEstadoPreparacaoDeck(EstadoPreparacaoDeck *deckSetup);

void DesenharTelaMenuPrincipal(int screenWidth, int screenHeight);
void DesenharTelaVitoria(int screenWidth, int screenHeight, int jogadorVencedor, int player1Score, int player2Score);

void DrawBottomMenu(int screenWidth, int screenHeight, int activePlayer, int playerCards[2][3], int playerMonsters[2][3]);
void DrawSelectionMenu(int screenWidth, int screenHeight, bool canPickMonster, int activePlayer);

void DrawScoreBoard(int screenWidth, int hudY, int player1Score, int player2Score, Texture2D iconP1, Texture2D iconP2);
void DrawBattleActivationPrompt(int screenWidth, int screenHeight, bool p0Present, bool p1Present, bool p0Activated, bool p1Activated);
void DrawPlacementSlotMenu(int screenWidth, int screenHeight, int activePlayer, const int availableSlots[3], int selectedSlot, bool selectingMonster);
void DrawGameHints(int screenWidth, int screenHeight, const char *placeMessage);
void DrawBattleFeedbackOverlay(int screenWidth, int screenHeight, const char *battleMessage, int battleResolveTimer, int placedFeedbackTimer);
void DrawPlacementPrecisionBar(int screenWidth, int screenHeight, int chosenSide, float cursorT, int activePlayer);

#endif