#ifndef CARD_H
#define CARD_H

#include <stdbool.h>

typedef struct {
    int owner; // 0 ou 1, -1 se vazio
    int slot;
} CardPlacement;

CardPlacement CriarCarta(int owner, int slot);
CardPlacement CartaVazia(void);
int DonoDaCarta(CardPlacement card);
int SlotDaCarta(CardPlacement card);
bool CartaEstaVazia(CardPlacement card);

#endif // fim de CARD_H
