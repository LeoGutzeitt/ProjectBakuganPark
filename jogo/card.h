#ifndef CARD_H
#define CARD_H

#include <stdbool.h>
#include "raylib.h"

typedef struct {
    int owner; // 0 ou 1, -1 se vazio
    int slot;
} CardPlacement;

CardPlacement CriarCarta(int owner, int slot);
CardPlacement CartaVazia(void);
int DonoDaCarta(CardPlacement card);
int SlotDaCarta(CardPlacement card);
bool CartaEstaVazia(CardPlacement card);

void ConfigureCardModel(Model *cardModel, Texture2D texture);
void DrawCardModelAt(const Model *cardModel, Vector3 position);
void DrawPlacedCard(Vector3 position, Color color);

#endif // fim de CARD_H
