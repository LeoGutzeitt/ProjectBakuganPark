#include "card.h"

void ConfigureCardModel(Model *cardModel, Texture2D texture)
{
    if (!cardModel) return;
    cardModel->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

void DrawCardModelAt(const Model *cardModel, Vector3 position)
{
    if (!cardModel) return;
    DrawModel(*cardModel, position, 1.0f, WHITE);
}

void DrawPlacedCard(Vector3 position, Color color)
{
    DrawCube(position, 1.8f, 0.05f, 2.4f, color);
    DrawCubeWires(position, 1.8f, 0.05f, 2.4f, BLACK);
}

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
