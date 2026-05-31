#ifndef MONSTER_H
#define MONSTER_H

#include <stdbool.h>
#include "raylib.h"

#define BAKUGAN_TYPE_COUNT 12
#define BAKUGAN_ELEMENT_COUNT 6

typedef enum {
    MONSTER_SIDE_LEFT = 0,
    MONSTER_SIDE_RIGHT = 1
} MonsterSide;

typedef enum {
    BAKUGAN_TYPE_VENTO_1 = 0,
    BAKUGAN_TYPE_VENTO_2 = 1,

    BAKUGAN_TYPE_AGUA_1 = 2,
    BAKUGAN_TYPE_AGUA_2 = 3,

    BAKUGAN_TYPE_TERRA_1 = 4,
    BAKUGAN_TYPE_TERRA_2 = 5,

    BAKUGAN_TYPE_FOGO_1 = 6,
    BAKUGAN_TYPE_FOGO_2 = 7,

    BAKUGAN_TYPE_ESCURO_1 = 8,
    BAKUGAN_TYPE_ESCURO_2 = 9,

    BAKUGAN_TYPE_LUZ_1 = 10,
    BAKUGAN_TYPE_LUZ_2 = 11
} BakuganType;

typedef enum {
    BAKUGAN_ELEMENT_AGUA = 0,
    BAKUGAN_ELEMENT_VENTO = 1,
    BAKUGAN_ELEMENT_TERRA = 2,
    BAKUGAN_ELEMENT_ESCURO = 3,
    BAKUGAN_ELEMENT_LUZ = 4,
    BAKUGAN_ELEMENT_FOGO = 5
} BakuganElement;

typedef struct {
    int owner;
    int slot;
    int power;
    int type;
    int element;
    int gx;
    int gz;
    MonsterSide side;
} MonsterPlacement;

typedef enum {
    MONSTER_ANIM_TRAJETORIA = 0,
    MONSTER_ANIM_BOLA_ELEMENTO = 1,
    MONSTER_ANIM_TRANSFORMACAO = 2
} MonsterAnimationPhase;

typedef struct MonsterAnimation {
    Vector3 position;
    Vector3 target;
    float rotation;
    bool active;
    MonsterSide side;

    int type;
    int element;
    int gx;
    int gz;

    int owner;
    int slot;

    MonsterAnimationPhase phase;
    int frame;
    int frameTimer;
} MonsterAnimation;

MonsterPlacement MakeMonsterPlacement(int owner, int slot, int type, int element);
MonsterPlacement EmptyMonsterPlacement(void);

int MonsterPlacementOwner(MonsterPlacement monster);
int SlotDoMonstro(MonsterPlacement monster);
int MonsterPlacementPower(MonsterPlacement monster);
int TipoDoMonstro(MonsterPlacement monster);
int ElementoDoMonstro(MonsterPlacement monster);

bool MonsterPlacementIsEmpty(MonsterPlacement monster);

const char *BakuganTypeName(int type);
const char *BakuganElementName(int element);

int BasePowerForType(int type);

void CarregarAnimacoesBakugan(void);
void DescarregarAnimacoesBakugan(void);

Texture2D ObterBakuganTexture(int type);
Texture2D ObterTexturaAnimacaoMonstro(const MonsterAnimation *animation);

MonsterAnimation CriarAnimacaoMonstro(void);

void IniciarAnimacaoMonstro(
    MonsterAnimation *animation,
    MonsterSide side,
    Vector3 start,
    Vector3 target,
    int gx,
    int gz,
    int owner,
    int slot,
    int type,
    int element
);

bool AtualizarAnimacaoMonstro(MonsterAnimation *animation);

void AtualizarTransformacaoMonstro(
    bool *transforming,
    int *transformTimer,
    int *transformStage
);

Texture2D SelecionarTexturaEstagioMonstro(
    Texture2D stage0,
    Texture2D stage1,
    Texture2D stage2,
    int transformStage
);

void DesenharMonstroBillboard(
    Camera3D camera,
    Texture2D texture,
    Vector3 position,
    Vector2 scale,
    int type,
    MonsterSide side
);

void DesenharAnimacaoMonstroBillboard(
    Camera3D camera,
    const MonsterAnimation *animation,
    Vector2 scale
);

#endif