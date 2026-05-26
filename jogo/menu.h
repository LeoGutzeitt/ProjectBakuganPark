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
    bool cardSelected[2][3];
    bool monsterSelected[2][3];
} DeckSetupState;

void InitDeckSetupState(DeckSetupState *state);
bool UpdateDeckSetupState(DeckSetupState *state);
void DrawMainMenuScreen(int screenWidth, int screenHeight);
void DrawDeckSetupScreen(int screenWidth, int screenHeight, const DeckSetupState *state);

#endif // MENU_H