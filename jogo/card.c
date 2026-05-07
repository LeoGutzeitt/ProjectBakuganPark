#include "card.h"

CardPlacement CriarCarta(int owner, int slot)
{
    CardPlacement card;
    card.owner = owner;
    card.slot = slot;
    return card;
}

CardPlacement CartaVazia(void)
{
    return CriarCarta(-1, -1);
}

int DonoDaCarta(CardPlacement card)
{
    return card.owner;
}

int SlotDaCarta(CardPlacement card)
{
    return card.slot;
}

bool CartaEstaVazia(CardPlacement card)
{
    return card.owner == -1;
}
