#ifndef MONSTER_H
#define MONSTER_H

#include <stdbool.h>
#include "raylib.h"

#define BAKUGAN_TYPE_COUNT 6
#define BAKUGAN_ELEMENT_COUNT 6

typedef enum {
    MONSTER_SIDE_LEFT = 0,
    MONSTER_SIDE_RIGHT = 1
} MonsterSide;

typedef enum {
    BAKUGAN_TYPE_DRAGAO = 0,
    BAKUGAN_TYPE_TIGRE = 1,
    BAKUGAN_TYPE_LOBO = 2,
    BAKUGAN_TYPE_GOLEM = 3,
    BAKUGAN_TYPE_SERPENTE = 4,
    BAKUGAN_TYPE_FENIX = 5
} BakuganType;

typedef enum {
    BAKUGAN_ELEMENT_FOGO = 0,
    BAKUGAN_ELEMENT_AGUA = 1,
    BAKUGAN_ELEMENT_TERRA = 2,
    BAKUGAN_ELEMENT_VENTO = 3,
    BAKUGAN_ELEMENT_SOMBRA = 4,
    BAKUGAN_ELEMENT_LUZ = 5
} BakuganElement;

typedef struct {
    int owner;
    int slot;
    int power;
    int type;
    int element;
    MonsterSide side;
} MonsterPlacement;

typedef struct MonsterAnimation {
    Vector3 position;
    Vector3 target;
    float rotation;
    bool active;
    MonsterSide side;
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

MonsterAnimation CriarAnimacaoMonstro(void);
void IniciarAnimacaoMonstro(MonsterAnimation *animation, MonsterSide side, Vector3 start, Vector3 target);
bool AtualizarAnimacaoMonstro(MonsterAnimation *animation);
void AtualizarTransformacaoMonstro(bool *transforming, int *transformTimer, int *transformStage);
Texture2D SelecionarTexturaEstagioMonstro(Texture2D stage0, Texture2D stage1, Texture2D stage2, int transformStage);
void DesenharMonstroBillboard(Camera3D camera, Texture2D texture, Vector3 position, Vector2 scale, MonsterSide side);

#endif // fim de MONSTER_H
