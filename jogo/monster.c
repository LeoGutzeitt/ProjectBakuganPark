#include "monster.h"
#include <math.h>

MonsterAnimation MonsterAnimationCreate(void)
{
    MonsterAnimation animation = {0};
    animation.side = MONSTER_SIDE_LEFT;
    return animation;
}

void MonsterAnimationStart(MonsterAnimation *animation, MonsterSide side, Vector3 start, Vector3 target)
{
    if (!animation) return;
    animation->side = side;
    animation->position = start;
    animation->target = target;
    animation->rotation = 0.0f;
    animation->active = true;
}

bool MonsterAnimationUpdate(MonsterAnimation *animation)
{
    if (!animation || !animation->active) return false;

    float speed = 6.0f * GetFrameTime();

    animation->position.x += (animation->target.x - animation->position.x) * speed;
    animation->position.y += (animation->target.y - animation->position.y) * speed;
    animation->position.z += (animation->target.z - animation->position.z) * speed;
    animation->rotation += 720.0f * GetFrameTime();

    float dx = animation->target.x - animation->position.x;
    float dy = animation->target.y - animation->position.y;
    float dz = animation->target.z - animation->position.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    if (dist < 0.05f) {
        animation->position = animation->target;
        animation->active = false;
        animation->rotation = 0.0f;
        return true;
    }

    return false;
}

void UpdateMonsterTransformation(bool *transforming, int *transformTimer, int *transformStage)
{
    if (!transforming || !transformTimer || !transformStage || !*transforming) return;

    (*transformTimer)++;

    if (*transformTimer > 60) {
        *transformStage = 1;
    }

    if (*transformTimer > 120) {
        *transformStage = 2;
    }

    if (*transformTimer > 180) {
        *transforming = false;
    }
}

Texture2D SelectMonsterStageTexture(Texture2D stage0, Texture2D stage1, Texture2D stage2, int transformStage)
{
    if (transformStage == 0) return stage0;
    if (transformStage == 1) return stage1;
    return stage2;
}

void DrawMonsterBillboard(Camera3D camera, Texture2D texture, Vector3 position, Vector2 scale, MonsterSide side)
{
    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };

    if (side == MONSTER_SIDE_RIGHT)
    {
        source.x = (float)texture.width;
        source.width = -(float)texture.width;
    }

    DrawBillboardRec(
        camera,
        texture,
        source,
        position,
        scale,
        WHITE
    );
}

MonsterPlacement MakeMonsterPlacement(int owner, int slot)
{
    MonsterPlacement monster;
    monster.owner = owner;
    monster.slot = slot;
    monster.power = slot + 1;
    monster.side = MONSTER_SIDE_LEFT;
    return monster;
}

MonsterPlacement EmptyMonsterPlacement(void)
{
    MonsterPlacement monster;
    monster.owner = -1;
    monster.slot = -1;
    monster.power = 0;
    monster.side = MONSTER_SIDE_LEFT;
    return monster;
}

int MonsterPlacementOwner(MonsterPlacement monster)
{
    return monster.owner;
}

int SlotDoMonstro(MonsterPlacement monster)
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
