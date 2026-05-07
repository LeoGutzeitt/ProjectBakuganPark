#include "monster.h"

MonsterPlacement MakeMonsterPlacement(int owner, int slot)
{
    MonsterPlacement monster;
    monster.owner = owner;
    monster.slot = slot;
    monster.power = slot + 1;
    return monster;
}

MonsterPlacement EmptyMonsterPlacement(void)
{
    MonsterPlacement monster;
    monster.owner = -1;
    monster.slot = -1;
    monster.power = 0;
    return monster;
}

int MonsterPlacementOwner(MonsterPlacement monster)
{
    return monster.owner;
}

int MonsterPlacementSlot(MonsterPlacement monster)
{
    return monster.slot;
}

int MonsterPlacementPower(MonsterPlacement monster)
{
    return monster.power;
}

bool MonsterPlacementIsEmpty(MonsterPlacement monster)
{
    return monster.owner == -1;
}
