#ifndef CARD_H
#define CARD_H

#include <stdbool.h>
#include "raylib.h"

#define CARD_TYPE_COUNT 6

typedef enum {
    CARD_TYPE_ATAQUE = 0,
    CARD_TYPE_DEFESA = 1,
    CARD_TYPE_ENERGIA = 2,
    CARD_TYPE_ARMADILHA = 3,
    CARD_TYPE_FOCO = 4,
    CARD_TYPE_SUPORTE = 5
} CardType;

typedef struct {
    int owner; // 0 ou 1, -1 se vazio
    int slot;
    int type;
} CardPlacement;

CardPlacement CriarCarta(int owner, int slot, int type);
CardPlacement CartaVazia(void);
int DonoDaCarta(CardPlacement card);
int SlotDaCarta(CardPlacement card);
int TipoDaCarta(CardPlacement card);
bool CartaEstaVazia(CardPlacement card);

const char *CardTypeName(int type);

int CardBonusForPortalCard(int cardSlot, int cardType, int element);

void ConfigureCardModel(Model *cardModel, Texture2D texture);
void DrawCardModelAt(const Model *cardModel, Vector3 position);
void DrawCardModelAtWithTexture(const Model *cardModel, Texture2D texture, Vector3 position);
void DrawCardModelExWithTexture(const Model *cardModel, Texture2D texture, Vector3 position, Vector3 axis, float angle, Vector3 scale);
void DrawPlacedCard(Vector3 position, Color color);

#endif // fim de CARD_H
