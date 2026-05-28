#include "menu.h"

#include "card.h"
#include "raylib.h"

static const char *CARD_RARITY_LABELS[3] = {
    "Bronze",
    "Prata",
    "Ouro"
};

static const char *CARD_NAMES[3][2] = {
    { "Bronze 1", "Bronze 2" },
    { "Prata 1",  "Prata 2"  },
    { "Ouro 1",   "Ouro 2"   }
};

static const char *BAKUGAN_TYPE_LABELS[6] = {
    "Dragao",
    "Tigre",
    "Lobo",
    "Golem",
    "Serpente",
    "Fenix"
};

static const char *ELEMENT_LABELS[6] = {
    "Pyros" ,
    "Aquos",
    "Subterra",
    "Ventus",
    "Darkus",
    "Lithus"
};

static const Color CARD_RARITY_COLORS[3] = {
    (Color){176, 110, 62, 255},  // Bronze
    (Color){168, 176, 190, 255}, // Prata
    (Color){236, 198, 62, 255}   // Ouro
};

static const Color BAKUGAN_TYPE_COLORS[6] = {
    (Color){200, 40, 40, 255},   // Dragao
    (Color){220, 140, 40, 255},  // Tigre
    (Color){140, 140, 160, 255}, // Lobo
    (Color){140, 100, 60, 255},  // Golem
    (Color){80, 180, 80, 255},   // Serpente
    (Color){240, 200, 60, 255}   // Fenix
};

static const Color ELEMENT_COLORS[6] = {
    (Color){230, 80, 40, 255},   // Fogo
    (Color){40, 130, 230, 255},  // Agua
    (Color){140, 100, 40, 255},  // Terra
    (Color){120, 200, 220, 255}, // Vento
    (Color){80, 80, 100, 255},   // Sombra
    (Color){250, 240, 180, 255}  // Luz
};

static bool PlayerDeckComplete(const EstadoPreparacaoDeck *state, int player)
{
    for (int slot = 0; slot < 3; slot++) {
        if (state->cardTypes[player][slot] < 0) return false;
        if (state->monsterTypes[player][slot] < 0) return false;
        if (state->monsterElements[player][slot] < 0) return false;
    }

    return true;
}

void InicializarEstadoPreparacaoDeck(EstadoPreparacaoDeck *state)
{
    if (!state) return;

    state->activePlayer = 0;
    state->selectedRow = 0;
    state->selectedSlot = 0;
    state->selectedChoice = 0;
    state->awaitingElementSlot = -1;

    for (int player = 0; player < 2; player++) {
        for (int slot = 0; slot < 3; slot++) {
            state->cardTypes[player][slot] = -1;
            state->monsterTypes[player][slot] = -1;
            state->monsterElements[player][slot] = -1;
        }
    }
}

bool AtualizarEstadoPreparacaoDeck(EstadoPreparacaoDeck *state)
{
    if (!state) return false;

    int player = state->activePlayer;
    if (PlayerDeckComplete(state, player)) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            if (player == 0) {
                state->activePlayer = 1;
                state->selectedRow = 0;
                state->selectedSlot = 0;
                state->selectedChoice = 0;
                state->awaitingElementSlot = -1;
            }
            return player == 1;
        }

        return false;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        state->selectedRow = (state->selectedRow + 4) % 5;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        state->selectedRow = (state->selectedRow + 1) % 5;
    }

    int optionCount = (state->selectedRow < 3) ? 2 : 6;

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        if (state->selectedRow < 3) {
            state->selectedChoice = (state->selectedChoice + optionCount - 1) % optionCount;
        } else {
            state->selectedSlot = (state->selectedSlot + 2) % 3;
        }
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        if (state->selectedRow < 3) {
            state->selectedChoice = (state->selectedChoice + 1) % optionCount;
        } else {
            state->selectedSlot = (state->selectedSlot + 1) % 3;
        }
    }

    if (IsKeyPressed(KEY_Q)) {
        state->selectedChoice = (state->selectedChoice + optionCount - 1) % optionCount;
    }
    if (IsKeyPressed(KEY_E)) {
        state->selectedChoice = (state->selectedChoice + 1) % optionCount;
    }

    for (int digit = 0; digit < optionCount; digit++) {
        if (IsKeyPressed(KEY_ONE + digit)) {
            state->selectedChoice = digit;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (state->selectedRow < 3) {
            state->cardTypes[player][state->selectedRow] = state->selectedRow * 2 + state->selectedChoice;
        } else if (state->selectedRow == 3) {
            // assign bakugan model to slot and immediately enter element selection
            state->monsterTypes[player][state->selectedSlot] = state->selectedChoice;
            state->awaitingElementSlot = state->selectedSlot;
            state->selectedChoice = 0;
            state->selectedRow = 4; // switch to element picking
        } else if (state->selectedRow == 4) {
            if (state->awaitingElementSlot >= 0) {
                state->monsterElements[player][state->awaitingElementSlot] = state->selectedChoice;
                // clear awaiting and return to bakugan selection
                state->awaitingElementSlot = -1;
                state->selectedRow = 3;
                // advance to next slot
                state->selectedSlot = (state->selectedSlot + 1) % 3;
            } else {
                state->monsterElements[player][state->selectedSlot] = state->selectedChoice;
            }
        }
    }

    return false;
}

static void DrawDeckSlot(int x, int y, int width, int height, const char *label, bool active, Color fillColor)
{
    Color outlineColor = active ? WHITE : DARKGRAY;
    Color textColor = BLACK;

    DrawRectangle(x, y, width, height, fillColor);
    DrawRectangleLines(x, y, width, height, outlineColor);
    DrawText(label, x + 10, y + height / 2 - 8, 16, textColor);
}

static Color GetRowColor(int row, int index)
{
    if (row < 3) return CARD_RARITY_COLORS[row];
    if (row == 3) return BAKUGAN_TYPE_COLORS[index];
    return ELEMENT_COLORS[index];
}

static const char *GetRowTitle(int row)
{
    if (row < 3) return CARD_RARITY_LABELS[row];
    if (row == 3) return "Escolha seu Bakugan";
    return "Elemento do Bakugan";
}

static const char *GetRowLabel(const EstadoPreparacaoDeck *state, int row, int index)
{
    if (row < 3) return CARD_NAMES[row][index];
    if (row == 3) return BAKUGAN_TYPE_LABELS[index];
    return ELEMENT_LABELS[index];
}

static const char *GetSelectedBakuganLabel(const EstadoPreparacaoDeck *state)
{
    int player = state->activePlayer;
    int slot = state->selectedSlot;

    if (state->monsterTypes[player][slot] >= 0) {
        return BAKUGAN_TYPE_LABELS[state->monsterTypes[player][slot]];
    }

    return "nenhum";
}

static void DrawStadiumTile(int x, int y, int w, int h, const char *label, Color color, bool selected, bool locked)
{
    Color fill = locked ? Fade(color, 0.35f) : color;
    Color border = selected ? RAYWHITE : Fade(BLACK, 0.4f);

    DrawRectangle(x, y, w, h, fill);
    DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, (float)w, (float)h }, selected ? 4.0f : 2.0f, border);
    DrawRectangle(x + 6, y + 6, 18, 18, selected ? RAYWHITE : BLACK);
    DrawText(label, x + 30, y + h / 2 - 9, 18, RAYWHITE);
}

static void DrawChoicePreview(int x, int y, int w, int h, const EstadoPreparacaoDeck *state, int row)
{
    int choice = state->selectedChoice;
    Color color = GetRowColor(row, choice);
    const char *title = GetRowTitle(row);
    const char *label = GetRowLabel(state, row, choice);

    DrawRectangle(x, y, w, h, (Color){24, 30, 48, 255});
    DrawRectangleLines(x, y, w, h, Fade(color, 0.9f));
    DrawCircle(x + w - 84, y + 80, 64, color);
    DrawCircleLines(x + w - 84, y + 80, 64, RAYWHITE);

    DrawText(title, x + 16, y + 16, 24, RAYWHITE);
    DrawText(TextFormat("%d - %s", choice + 1, label), x + 16, y + 58, 30, color);

    if (row < 3) {
        DrawText(
            TextFormat("Escolha 1 das 2 cartas %s", CARD_RARITY_LABELS[row]),
            x + 16,
            y + 104,
            18,
            SKYBLUE
        );
        DrawText(
            TextFormat("Bakugan já escolhido: %s", GetSelectedBakuganLabel(state)),
            x + 16,
            y + 132,
            18,
            RAYWHITE
        );
    } else if (row == 3) {
        DrawText(
            TextFormat("Bakugan: %s", GetSelectedBakuganLabel(state)),
            x + 16,
            y + 104,
            18,
            SKYBLUE
        );
        DrawText(
            TextFormat("Elemento para o slot %d", state->awaitingElementSlot + 1),
            x + 16,
            y + 132,
            18,
            RAYWHITE
        );
    } else {
        DrawText("Escolha 3 cartas para liberar o deck", x + 16, y + 104, 18, RAYWHITE);
    }

    DrawText("ENTER confirma | W/S troca categoria | A/D/Q/E mudam a escolha", x + 16, y + h - 32, 16, Fade(RAYWHITE, 0.8f));
}

static void DrawPlayerPanel(const EstadoPreparacaoDeck *state, int player, int panelX, int panelY, int panelW, int panelH)
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
        TextFormat("%d/3", 3 - ((state->cardTypes[player][0] < 0) + (state->cardTypes[player][1] < 0) + (state->cardTypes[player][2] < 0))),
        panelX + panelW - 74,
        panelY + 58,
        18,
        border
    );

    DrawText(
        TextFormat("%d/6", 6 - ((state->monsterTypes[player][0] < 0) + (state->monsterTypes[player][1] < 0) + (state->monsterTypes[player][2] < 0) + (state->monsterElements[player][0] < 0) + (state->monsterElements[player][1] < 0) + (state->monsterElements[player][2] < 0))),
        panelX + panelW - 74,
        panelY + 162,
        18,
        border
    );

    int slotWidth = 170;
    int slotHeight = 52;
    int slotGap = 14;
    int rowX = panelX + 18;
    int firstSlotX = rowX + 90;
    int cardY = panelY + 84;
    int monsterY = panelY + 188;

    for (int slot = 0; slot < 3; slot++) {
        int x = firstSlotX + slot * (slotWidth + slotGap);
        bool cardActive = active && state->selectedRow == slot;
        bool monsterTypeActive = active && state->selectedRow == 3 && state->selectedSlot == slot;
        bool monsterElementActive = active && state->selectedRow == 4 && state->selectedSlot == slot;

        DrawDeckSlot(
            x,
            cardY,
            slotWidth,
            slotHeight,
            state->cardTypes[player][slot] >= 0 ? CARD_NAMES[slot][state->cardTypes[player][slot] % 2] : CARD_RARITY_LABELS[slot],
            cardActive,
            cardActive ? CARD_RARITY_COLORS[slot] : (Color){240, 240, 240, 255}
        );

        const char *monsterLabel;
        if (state->monsterTypes[player][slot] >= 0) {
            if (state->monsterElements[player][slot] >= 0) {
                monsterLabel = TextFormat("%s / %s", BAKUGAN_TYPE_LABELS[state->monsterTypes[player][slot]], ELEMENT_LABELS[state->monsterElements[player][slot]]);
            } else {
                monsterLabel = TextFormat("%s / ?", BAKUGAN_TYPE_LABELS[state->monsterTypes[player][slot]]);
            }
        } else {
            monsterLabel = TextFormat("Bakugan %d", slot + 1);
        }

        bool awaitingThis = (state->awaitingElementSlot == slot && state->activePlayer == player);

        DrawDeckSlot(
            x,
            monsterY,
            slotWidth,
            slotHeight,
            monsterLabel,
            monsterTypeActive || monsterElementActive || awaitingThis,
            (monsterTypeActive || monsterElementActive || awaitingThis) ? (player == 0 ? SKYBLUE : RED) : (Color){240, 240, 240, 255}
        );
    }

    if (PlayerDeckComplete(state, player)) {
        DrawText("Pronto para continuar", panelX + 18, panelY + panelH - 34, 18, DARKGREEN);
    } else {
        DrawText("Use setas e ENTER/ESPACO para marcar", panelX + 18, panelY + panelH - 34, 16, DARKGRAY);
    }
}

static void DrawChoicePalette(int screenWidth, int screenHeight, const EstadoPreparacaoDeck *state)
{
    int panelH = 260;
    int panelY = screenHeight - panelH - 72;
    DrawRectangle(0, panelY, screenWidth, panelH, (Color){18, 24, 36, 255});
    DrawRectangle(0, panelY, screenWidth, 4, (Color){220, 220, 220, 60});

    int row = state->selectedRow;
    int gridX = 44;
    int gridY = panelY + 54;
    int tileW = (row < 3) ? 320 : 270;
    int tileH = 54;
    int gapX = 18;
    int gapY = 14;
    int previewX = screenWidth - 520;
    int previewY = panelY + 22;
    int previewW = 450;
    int previewH = 210;

    DrawText(GetRowTitle(row), gridX, panelY + 14, 24, RAYWHITE);
    DrawText("Escolha visual estilo Stadium", gridX + 280, panelY + 16, 16, SKYBLUE);

    int optionCount = (row < 3) ? 2 : 6;
    for (int i = 0; i < optionCount; i++) {
        int col = (row < 3) ? i : (i % 3);
        int r = (row < 3) ? 0 : (i / 3);
        int x = gridX + col * (tileW + gapX);
        int y = gridY + r * (tileH + gapY);
        bool selected = (state->selectedChoice == i);
        bool locked = false;

        DrawStadiumTile(x, y, tileW, tileH, TextFormat("%d  %s", i + 1, GetRowLabel(state, row, i)), GetRowColor(row, i), selected, locked);
    }

    DrawChoicePreview(previewX, previewY, previewW, previewH, state, row);
}

void DesenharTelaMenuPrincipal(int screenWidth, int screenHeight)
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

void DesenharTelaPreparacaoDeck(int screenWidth, int screenHeight, const EstadoPreparacaoDeck *state)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){238, 242, 248, 255});
    DrawRectangle(0, 0, screenWidth, 90, (Color){30, 37, 56, 255});

    const char *title = "Montagem de Decks";
    DrawText(title, 40, 22, 34, RAYWHITE);
    DrawText("Cada jogador escolhe 1 Bronze, 1 Prata e 1 Ouro, depois 3 Bakugans e seus elementos", 40, 56, 18, SKYBLUE);
    DrawText("As cartas agora sao 6 unicas: 2 bronze, 2 prata e 2 ouro.", 40, 76, 16, RAYWHITE);

    int panelW = (screenWidth - 120) / 2;
    int panelH = 320;
    int panelY = 140;

    DrawPlayerPanel(state, 0, 40, panelY, panelW, panelH);
    DrawPlayerPanel(state, 1, 80 + panelW, panelY, panelW, panelH);

    int infoY = panelY + panelH + 34;
    if (PlayerDeckComplete(state, state->activePlayer)) {
        DrawText("Deck completo. Pressione ESPACO para passar ao proximo jogador.", 40, infoY, 22, DARKGREEN);
    } else {
        DrawText("W/S troca entre Bronze, Prata, Ouro, Bakugan e Elemento", 40, infoY, 22, DARKGRAY);
        DrawText("A/D ou Q/E mudam a escolha dentro da categoria atual.", 40, infoY + 30, 20, DARKGRAY);
    }

    DrawChoicePalette(screenWidth, screenHeight, state);
}

void DesenharTelaVitoria(int screenWidth, int screenHeight, int vencedorJogador, int pontuacaoJogador1, int pontuacaoJogador2)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){12, 18, 30, 255});
    DrawCircleGradient(screenWidth / 2, screenHeight / 2 - 20, 520.0f, (Color){70, 110, 180, 180}, (Color){12, 18, 30, 0});
    DrawCircleGradient(screenWidth / 2, screenHeight / 2 + 120, 220.0f, (Color){240, 180, 50, 120}, (Color){12, 18, 30, 0});

    const char *winnerText = (vencedorJogador == 0) ? "P1 venceu a partida" : "P2 venceu a partida";
    const Color winnerColor = (vencedorJogador == 0) ? BLUE : RED;

    DrawText("VITORIA", screenWidth / 2 - MeasureText("VITORIA", 64) / 2, 110, 64, RAYWHITE);
    DrawRectangle(screenWidth / 2 - 210, 210, 420, 170, (Color){255, 255, 255, 18});
    DrawRectangleLines(screenWidth / 2 - 210, 210, 420, 170, winnerColor);
    DrawText(winnerText, screenWidth / 2 - MeasureText(winnerText, 34) / 2, 256, 34, winnerColor);
    DrawText(TextFormat("Placar final: P1 %d  -  P2 %d", pontuacaoJogador1, pontuacaoJogador2), screenWidth / 2 - 140, 310, 22, RAYWHITE);
    DrawText("Primeiro a chegar em 3 pontos ganhou", screenWidth / 2 - 160, 342, 18, SKYBLUE);

    DrawRectangle(screenWidth / 2 - 250, 432, 500, 170, (Color){22, 30, 48, 220});
    DrawRectangleLines(screenWidth / 2 - 250, 432, 500, 170, Fade(winnerColor, 0.9f));
    DrawText("S  Estatisticas da partida", screenWidth / 2 - 200, 470, 24, RAYWHITE);
    DrawText("H  Ir para a tela inicial", screenWidth / 2 - 200, 512, 24, RAYWHITE);
    DrawText("ENTER  Voltar para o menu inicial", screenWidth / 2 - 200, 552, 20, SKYBLUE);
}
