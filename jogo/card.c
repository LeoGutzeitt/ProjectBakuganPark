#include "card.h"
#include "monster.h"

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

void DrawCardModelAtWithTexture(const Model *cardModel, Texture2D texture, Vector3 position)
{
    if (!cardModel) return;

    Model card = *cardModel;
    card.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    DrawModel(card, position, 1.0f, WHITE);
}

void DrawCardModelExWithTexture(const Model *cardModel, Texture2D texture, Vector3 position, Vector3 axis, float angle, Vector3 scale)
{
    if (!cardModel) return;

    Model card = *cardModel;
    card.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    DrawModelEx(card, position, axis, angle, scale, WHITE);
}

void DrawPlacedCard(Vector3 position, Color color)
{
    DrawCube(position, 1.8f, 0.05f, 2.4f, color);
    DrawCubeWires(position, 1.8f, 0.05f, 2.4f, BLACK);
}

int CardBonusForPortalCard(int cardSlot, int cardType, int element)
{
    // cardSlot: 0 = bronze, 1 = silver, 2 = gold
    // cardType: 0 = card 1, 1 = card 2, 2 = card 3
    // elementos seguindo esta ordem: fogo, agua, terra, luz, escuro, vento
    static const int bonus[3][3][BAKUGAN_ELEMENT_COUNT] = {
        {
            { 20, 80, 140, 110, 60, 50 },
            { 50, 60, 100, 80, 150, 150 },
            { 140, 80, 50, 20, 60, 80 }
        },
        {
            { 140, 120, 80, 60, 40, 20 },
            { 150, 30, 100, 130, 10, 120 },
            { 80, 120, 140, 30, 50, 180 }
        },
        {
            { 100, 90, 150, 200, 110, 50 },
            { 150, 90, 110, 130, 70, 120 },
            { 150, 170, 100, 40, 100, 110 }
        }
    };

    if (cardSlot < 0 || cardSlot >= 3) return 0;
    if (cardType < 0 || cardType >= 3) return 0;
    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT) return 0;
    return bonus[cardSlot][cardType][element];
}

const char *CardTypeName(int type)
{
    static const char *names[CARD_TYPE_COUNT] = {
        "Ataque",
        "Defesa",
        "Energia",
        "Armadilha",
        "Foco",
        "Suporte"
    };

    if (type < 0 || type >= CARD_TYPE_COUNT) return "Desconhecida";
    return names[type];
}

CardPlacement CriarCarta(int owner, int slot, int type)
{
    CardPlacement card;
    card.owner = owner;
    card.slot = slot;
    card.type = type;
    return card;
}

CardPlacement CartaVazia(void)
{
    return CriarCarta(-1, -1, -1);
}

int DonoDaCarta(CardPlacement card)
{
    return card.owner;
}

int SlotDaCarta(CardPlacement card)
{
    return card.slot;
}

int TipoDaCarta(CardPlacement card)
{
    return card.type;
}

bool CartaEstaVazia(CardPlacement card)
{
    return card.owner == -1;
}
