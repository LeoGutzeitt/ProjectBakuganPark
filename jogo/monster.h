#ifndef MONSTER_H
#define MONSTER_H

#include <stdbool.h>

typedef struct {
    int owner;
    int slot;
    int power;
} MonsterPlacement;

MonsterPlacement MakeMonsterPlacement(int owner, int slot);
MonsterPlacement EmptyMonsterPlacement(void);
int MonsterPlacementOwner(MonsterPlacement monster);
int MonsterPlacementSlot(MonsterPlacement monster);
int MonsterPlacementPower(MonsterPlacement monster);
bool MonsterPlacementIsEmpty(MonsterPlacement monster);

#endif // fim de MONSTER_H
