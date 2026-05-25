#ifndef MONSTER_H
#define MONSTER_H

#include <stdbool.h>
#include "raylib.h"

typedef struct {
    int owner;
    int slot;
    int power;
} MonsterPlacement;

typedef struct MonsterAnimation {
    Vector3 position;
    Vector3 target;
    float rotation;
    bool active;
} MonsterAnimation;

MonsterPlacement MakeMonsterPlacement(int owner, int slot);
MonsterPlacement EmptyMonsterPlacement(void);
int MonsterPlacementOwner(MonsterPlacement monster);
int SlotDoMonstro(MonsterPlacement monster);
int MonsterPlacementPower(MonsterPlacement monster);
bool MonsterPlacementIsEmpty(MonsterPlacement monster);

MonsterAnimation MonsterAnimationCreate(void);
void MonsterAnimationStart(MonsterAnimation *animation, Vector3 start, Vector3 target);
bool MonsterAnimationUpdate(MonsterAnimation *animation);
void UpdateMonsterTransformation(bool *transforming, int *transformTimer, int *transformStage);
Texture2D SelectMonsterStageTexture(Texture2D stage0, Texture2D stage1, Texture2D stage2, int transformStage);
void DrawMonsterBillboard(Camera3D camera, Texture2D texture, Vector3 position, Vector2 scale, Vector2 origin, float rotationDeg);

#endif // fim de MONSTER_H
