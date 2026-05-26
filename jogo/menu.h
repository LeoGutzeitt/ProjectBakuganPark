#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    APP_SCREEN_MAIN_MENU = 0,
    APP_SCREEN_DECK_SETUP = 1,
    APP_SCREEN_BATTLE = 2
} AppScreen;

typedef struct {
    int activePlayer;
    int selectedRow;
    int selectedSlot;
    int selectedChoice;
    int cardTypes[2][3];
    int monsterTypes[2][3];
    int monsterElements[2][3];
    int awaitingElementSlot; // -1 when not awaiting
} DeckSetupState;

void InitDeckSetupState(DeckSetupState *state);
bool UpdateDeckSetupState(DeckSetupState *state);
void DrawMainMenuScreen(int screenWidth, int screenHeight);
void DrawDeckSetupScreen(int screenWidth, int screenHeight, const DeckSetupState *state);

#endif // MENU_H