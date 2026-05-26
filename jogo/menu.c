#include "menu.h"

#include "raylib.h"

static int CountSelectedSlots(const bool slots[3])
{
    int count = 0;
    for (int i = 0; i < 3; i++) {
        if (slots[i]) count++;
    }
    return count;
}

static bool PlayerDeckComplete(const DeckSetupState *state, int player)
{
    return CountSelectedSlots(state->cardSelected[player]) == 3 &&
           CountSelectedSlots(state->monsterSelected[player]) == 3;
}

void InitDeckSetupState(DeckSetupState *state)
{
    if (!state) return;

    state->activePlayer = 0;
    state->selectedRow = 0;
    state->selectedSlot = 0;

    for (int player = 0; player < 2; player++) {
        for (int slot = 0; slot < 3; slot++) {
            state->cardSelected[player][slot] = false;
            state->monsterSelected[player][slot] = false;
        }
    }
}

bool UpdateDeckSetupState(DeckSetupState *state)
{
    if (!state) return false;

    int player = state->activePlayer;
    if (PlayerDeckComplete(state, player)) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            if (player == 0) {
                state->activePlayer = 1;
                state->selectedRow = 0;
                state->selectedSlot = 0;
            }
            return player == 1;
        }

        return false;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        state->selectedRow = 0;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        state->selectedRow = 1;
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        state->selectedSlot = (state->selectedSlot + 2) % 3;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        state->selectedSlot = (state->selectedSlot + 1) % 3;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (state->selectedRow == 0) {
            state->cardSelected[player][state->selectedSlot] = true;
        } else {
            state->monsterSelected[player][state->selectedSlot] = true;
        }
    }

    return false;
}

static void DrawDeckSlot(int x, int y, int size, const char *label, bool selected, bool active, Color fillColor)
{
    Color outlineColor = active ? WHITE : DARKGRAY;
    Color textColor = selected ? WHITE : BLACK;

    DrawRectangle(x, y, size, size, selected ? fillColor : (Color){235, 235, 235, 255});
    DrawRectangleLines(x, y, size, size, outlineColor);

    if (selected) {
        DrawText("OK", x + 10, y + 8, 16, textColor);
    }

    DrawText(label, x + size / 2 - MeasureText(label, 14) / 2, y + size - 22, 14, textColor);
}

static void DrawPlayerPanel(const DeckSetupState *state, int player, int panelX, int panelY, int panelW, int panelH)
{
    bool active = (state->activePlayer == player);
    Color border = active ? (player == 0 ? BLUE : RED) : GRAY;
    Color background = active ? (Color){245, 245, 245, 255} : (Color){225, 225, 225, 255};

    DrawRectangle(panelX, panelY, panelW, panelH, background);
    DrawRectangleLines(panelX, panelY, panelW, panelH, border);
    DrawRectangle(panelX, panelY, panelW, 6, border);

    DrawText(TextFormat("P%d", player + 1), panelX + 18, panelY + 14, 28, border);

    DrawText("Cartas", panelX + 18, panelY + 56, 20, BLACK);
    DrawText("Bakugans", panelX + 18, panelY + 160, 20, BLACK);

    DrawText(
        TextFormat("%d/3", CountSelectedSlots(state->cardSelected[player])),
        panelX + panelW - 74,
        panelY + 58,
        18,
        border
    );

    DrawText(
        TextFormat("%d/3", CountSelectedSlots(state->monsterSelected[player])),
        panelX + panelW - 74,
        panelY + 162,
        18,
        border
    );

    int slotSize = 66;
    int slotGap = 14;
    int rowX = panelX + 18;
    int firstSlotX = rowX + 90;
    int cardY = panelY + 84;
    int monsterY = panelY + 188;

    for (int slot = 0; slot < 3; slot++) {
        int x = firstSlotX + slot * (slotSize + slotGap);
        bool cardActive = active && state->selectedRow == 0 && state->selectedSlot == slot;
        bool monsterActive = active && state->selectedRow == 1 && state->selectedSlot == slot;

        DrawDeckSlot(
            x,
            cardY,
            slotSize,
            TextFormat("%d", slot + 1),
            state->cardSelected[player][slot],
            cardActive,
            ORANGE
        );

        DrawDeckSlot(
            x,
            monsterY,
            slotSize,
            TextFormat("%d", slot + 1),
            state->monsterSelected[player][slot],
            monsterActive,
            player == 0 ? SKYBLUE : RED
        );
    }

    if (PlayerDeckComplete(state, player)) {
        DrawText("Pronto para continuar", panelX + 18, panelY + panelH - 34, 18, DARKGREEN);
    } else {
        DrawText("Use setas e ENTER/ESPACO para marcar", panelX + 18, panelY + panelH - 34, 16, DARKGRAY);
    }
}

void DrawMainMenuScreen(int screenWidth, int screenHeight)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){20, 26, 40, 255});
    DrawCircleGradient(screenWidth / 2, screenHeight / 2, 560.0f, (Color){40, 90, 150, 180}, (Color){20, 26, 40, 0});

    const char *title = "Bakugan Park";
    const char *subtitle = "Monte os decks e entre na batalha";
    const char *hint = "Pressione ENTER para comecar";

    DrawText(title, screenWidth / 2 - MeasureText(title, 72) / 2, screenHeight / 2 - 150, 72, RAYWHITE);
    DrawText(subtitle, screenWidth / 2 - MeasureText(subtitle, 28) / 2, screenHeight / 2 - 58, 28, SKYBLUE);
    DrawRectangle(screenWidth / 2 - 210, screenHeight / 2 + 20, 420, 88, (Color){255, 255, 255, 18});
    DrawRectangleLines(screenWidth / 2 - 210, screenHeight / 2 + 20, 420, 88, (Color){255, 255, 255, 90});
    DrawText(hint, screenWidth / 2 - MeasureText(hint, 24) / 2, screenHeight / 2 + 48, 24, YELLOW);
}

void DrawDeckSetupScreen(int screenWidth, int screenHeight, const DeckSetupState *state)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){238, 242, 248, 255});
    DrawRectangle(0, 0, screenWidth, 90, (Color){30, 37, 56, 255});

    const char *title = "Montagem de Decks";
    DrawText(title, 40, 22, 34, RAYWHITE);
    DrawText("Cada jogador escolhe 3 cartas e 3 bakugans antes da batalha", 40, 56, 18, SKYBLUE);

    int panelW = (screenWidth - 120) / 2;
    int panelH = 320;
    int panelY = 140;

    DrawPlayerPanel(state, 0, 40, panelY, panelW, panelH);
    DrawPlayerPanel(state, 1, 80 + panelW, panelY, panelW, panelH);

    int infoY = panelY + panelH + 34;
    if (PlayerDeckComplete(state, state->activePlayer)) {
        DrawText("Deck completo. Pressione ESPACO para passar ao proximo jogador.", 40, infoY, 22, DARKGREEN);
    } else {
        DrawText("Setas movem, ENTER marca a escolha atual.", 40, infoY, 22, DARKGRAY);
        DrawText("Quando os 6 slots estiverem marcados, avance para o outro jogador.", 40, infoY + 30, 20, DARKGRAY);
    }
}
