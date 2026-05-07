#include "card.h"

CardPlacement MakeCardPlacement(int owner, int slot)
{
    CardPlacement card;
    card.owner = owner;
    card.slot = slot;
    return card;
}

CardPlacement EmptyCardPlacement(void)
{
    return MakeCardPlacement(-1, -1);
}

int CardPlacementOwner(CardPlacement card)
{
    return card.owner;
}

int CardPlacementSlot(CardPlacement card)
{
    return card.slot;
}

bool CardPlacementIsEmpty(CardPlacement card)
{
    return card.owner == -1;
}
