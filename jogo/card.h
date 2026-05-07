#ifndef CARD_H
#define CARD_H

#include <stdbool.h>

typedef struct {
    int owner; // 0 ou 1, -1 se vazio
    int slot;
} CardPlacement;

CardPlacement MakeCardPlacement(int owner, int slot);
CardPlacement EmptyCardPlacement(void);
int CardPlacementOwner(CardPlacement card);
int CardPlacementSlot(CardPlacement card);
bool CardPlacementIsEmpty(CardPlacement card);

#endif // fim de CARD_H
