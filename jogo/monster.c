#include "monster.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_JSON_FRAMES 64

typedef struct {
    Texture2D texture;
    Rectangle frames[MAX_JSON_FRAMES];
    int frameCount;
} SpriteJsonAnimation;

static Texture2D bakuganFinalTextures[BAKUGAN_TYPE_COUNT];
static Texture2D bolaElementoTextures[BAKUGAN_ELEMENT_COUNT];
static Texture2D transformTextures[BAKUGAN_TYPE_COUNT];

static SpriteJsonAnimation rolagemAnimations[BAKUGAN_ELEMENT_COUNT];

static const char *ElementFileName(int element)
{
    static const char *names[BAKUGAN_ELEMENT_COUNT] = {
        "agua",
        "vento",
        "terra",
        "escuro",
        "luz",
        "fogo"
    };

    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
        return "agua";

    return names[element];
}

static int ElementBallNumber(int element)
{
    static const int numbers[BAKUGAN_ELEMENT_COUNT] = {
        1,
        2,
        3,
        4,
        5,
        6
    };

    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
        return 1;

    return numbers[element];
}

static const char *TypeFileName(int type)
{
    static const char *names[BAKUGAN_TYPE_COUNT] = {
        "vento1",
        "vento2",
        "agua1",
        "agua2",
        "terra1",
        "terra2",
        "fogo1",
        "fogo2",
        "escuro1",
        "escuro2",
        "luz1",
        "luz2"
    };

    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        return "vento1";

    return names[type];
}

const char *BakuganTypeName(int type)
{
    static const char *names[BAKUGAN_TYPE_COUNT] = {
        "Vento 1",
        "Vento 2",
        "Agua 1",
        "Agua 2",
        "Terra 1",
        "Terra 2",
        "Fogo 1",
        "Fogo 2",
        "Escuro 1",
        "Escuro 2",
        "Luz 1",
        "Luz 2"
    };

    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        return "Desconhecido";

    return names[type];
}

const char *BakuganElementName(int element)
{
    static const char *names[BAKUGAN_ELEMENT_COUNT] = {
        "Agua",
        "Vento",
        "Terra",
        "Escuro",
        "Luz",
        "Fogo"
    };

    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
        return "Desconhecido";

    return names[element];
}

static int LerInteiroJson(const char *inicio, const char *chave)
{
    const char *p = strstr(inicio, chave);

    if (!p)
        return 0;

    p = strchr(p, ':');

    if (!p)
        return 0;

    int valor = 0;
    sscanf(p + 1, "%d", &valor);

    return valor;
}

static void LerImagemDoJson(const char *jsonText, char *outPath, int outSize, const char *fallbackPath)
{
    const char *p = strstr(jsonText, "\"image\"");

    if (!p)
    {
        snprintf(outPath, outSize, "%s", fallbackPath);
        return;
    }

    p = strchr(p, ':');

    if (!p)
    {
        snprintf(outPath, outSize, "%s", fallbackPath);
        return;
    }

    p = strchr(p, '"');

    if (!p)
    {
        snprintf(outPath, outSize, "%s", fallbackPath);
        return;
    }

    p++;

    const char *fim = strchr(p, '"');

    if (!fim)
    {
        snprintf(outPath, outSize, "%s", fallbackPath);
        return;
    }

    char nomeImagem[128] = {0};
    int len = (int)(fim - p);

    if (len >= (int)sizeof(nomeImagem))
        len = sizeof(nomeImagem) - 1;

    strncpy(nomeImagem, p, len);
    nomeImagem[len] = '\0';

    if (strstr(nomeImagem, "img/") == nomeImagem)
        snprintf(outPath, outSize, "%s", nomeImagem);
    else
        snprintf(outPath, outSize, "img/%s", nomeImagem);
}

static void CarregarAnimacaoJson(SpriteJsonAnimation *anim, const char *jsonPath, const char *fallbackImagePath)
{
    if (!anim)
        return;

    anim->frameCount = 0;
    anim->texture.id = 0;

    char *jsonText = LoadFileText(jsonPath);

    if (!jsonText)
    {
        // try alternative path prefixed with 'jogo/'
        char altJson[256];
        snprintf(altJson, sizeof(altJson), "jogo/%s", jsonPath);
        jsonText = LoadFileText(altJson);
    }

    if (!jsonText)
    {
        anim->texture = LoadTexture(fallbackImagePath);
        if (anim->texture.id == 0)
        {
            char alt[256];
            snprintf(alt, sizeof(alt), "jogo/%s", fallbackImagePath);
            anim->texture = LoadTexture(alt);
        }

        if (anim->texture.id != 0)
        {
            anim->frames[0] = (Rectangle){
                0,
                0,
                (float)anim->texture.width,
                (float)anim->texture.height
            };

            anim->frameCount = 1;
        }

        return;
    }

    char imagePath[160];
    LerImagemDoJson(jsonText, imagePath, sizeof(imagePath), fallbackImagePath);

    anim->texture = LoadTexture(imagePath);
    if (anim->texture.id == 0)
    {
        char alt[256];
        snprintf(alt, sizeof(alt), "jogo/%s", imagePath);
        anim->texture = LoadTexture(alt);
    }

    const char *p = jsonText;

    while ((p = strstr(p, "\"frame\"")) != NULL && anim->frameCount < MAX_JSON_FRAMES)
    {
        int x = LerInteiroJson(p, "\"x\"");
        int y = LerInteiroJson(p, "\"y\"");
        int w = LerInteiroJson(p, "\"w\"");
        int h = LerInteiroJson(p, "\"h\"");

        if (w > 0 && h > 0)
        {
            anim->frames[anim->frameCount] = (Rectangle){
                (float)x,
                (float)y,
                (float)w,
                (float)h
            };

            anim->frameCount++;
        }

        p += 7;
    }

    if (anim->frameCount == 0 && anim->texture.id != 0)
    {
        anim->frames[0] = (Rectangle){
            0,
            0,
            (float)anim->texture.width,
            (float)anim->texture.height
        };

        anim->frameCount = 1;
    }

    UnloadFileText(jsonText);
}

void CarregarAnimacoesBakugan(void)
{
    char path[160];
    char fallback[160];

    for (int type = 0; type < BAKUGAN_TYPE_COUNT; type++)
    {
        snprintf(path, sizeof(path), "img/%s.png", TypeFileName(type));
        bakuganFinalTextures[type] = LoadTexture(path);
        if (bakuganFinalTextures[type].id == 0)
        {
            char alt[256];
            snprintf(alt, sizeof(alt), "jogo/%s", path);
            bakuganFinalTextures[type] = LoadTexture(alt);
        }

        snprintf(path, sizeof(path), "img/%s.t.png", TypeFileName(type));
        transformTextures[type] = LoadTexture(path);
        if (transformTextures[type].id == 0)
        {
            char altt[256];
            snprintf(altt, sizeof(altt), "jogo/%s", path);
            transformTextures[type] = LoadTexture(altt);
        }
    }

    for (int element = 0; element < BAKUGAN_ELEMENT_COUNT; element++)
    {
        snprintf(
            path,
            sizeof(path),
            "img/%d_bola_%s.png",
            ElementBallNumber(element),
            ElementFileName(element)
        );

        bolaElementoTextures[element] = LoadTexture(path);
        if (bolaElementoTextures[element].id == 0)
        {
            char altb[256];
            snprintf(altb, sizeof(altb), "jogo/%s", path);
            bolaElementoTextures[element] = LoadTexture(altb);
        }

        if (bolaElementoTextures[element].id == 0)
            printf("ERRO bola elemento: %s\n", path);

        snprintf(path, sizeof(path), "img/rolagem_%s.json", ElementFileName(element));
        snprintf(fallback, sizeof(fallback), "img/rolagem_%s.png", ElementFileName(element));

        CarregarAnimacaoJson(&rolagemAnimations[element], path, fallback);
        printf("[ANIM] element=%s bola_id=%d anim_tex_id=%d frames=%d\n",
            ElementFileName(element),
            bolaElementoTextures[element].id,
            rolagemAnimations[element].texture.id,
            rolagemAnimations[element].frameCount
        );
    }
}

void DescarregarAnimacoesBakugan(void)
{
    for (int type = 0; type < BAKUGAN_TYPE_COUNT; type++)
    {
        if (bakuganFinalTextures[type].id != 0)
            UnloadTexture(bakuganFinalTextures[type]);

        if (transformTextures[type].id != 0)
            UnloadTexture(transformTextures[type]);
    }

    for (int element = 0; element < BAKUGAN_ELEMENT_COUNT; element++)
    {
        if (bolaElementoTextures[element].id != 0)
            UnloadTexture(bolaElementoTextures[element]);

        if (rolagemAnimations[element].texture.id != 0)
            UnloadTexture(rolagemAnimations[element].texture);
    }
}

Texture2D ObterBakuganTexture(int type)
{
    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        type = 0;

    return bakuganFinalTextures[type];
}

Texture2D ObterTexturaAnimacaoMonstro(const MonsterAnimation *animation)
{
    if (!animation)
        return ObterBakuganTexture(0);

    int type = animation->type;
    int element = animation->element;

    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        type = 0;

    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
        element = BAKUGAN_ELEMENT_FOGO;

    if (animation->phase == MONSTER_ANIM_TRAJETORIA)
    {
        SpriteJsonAnimation *anim = &rolagemAnimations[element];

        if (anim->texture.id != 0)
            return anim->texture;

        if (bolaElementoTextures[element].id != 0)
            return bolaElementoTextures[element];

        return ObterBakuganTexture(type);
    }

    if (animation->phase == MONSTER_ANIM_BOLA_ELEMENTO)
    {
        if (bolaElementoTextures[element].id != 0)
            return bolaElementoTextures[element];

        return ObterBakuganTexture(type);
    }

    if (animation->phase == MONSTER_ANIM_TRANSFORMACAO)
    {
        if (transformTextures[type].id != 0)
            return transformTextures[type];

        return ObterBakuganTexture(type);
    }

    return ObterBakuganTexture(type);
}

MonsterAnimation CriarAnimacaoMonstro(void)
{
    MonsterAnimation animation = {0};

    animation.side = MONSTER_SIDE_LEFT;
    animation.active = false;
    animation.type = -1;
    animation.element = -1;
    animation.phase = MONSTER_ANIM_TRAJETORIA;
    animation.frame = 0;
    animation.frameTimer = 0;
    animation.gx = -1;
    animation.gz = -1;
    animation.owner = -1;
    animation.slot = -1;

    return animation;
}

void IniciarAnimacaoMonstro(MonsterAnimation *animation, MonsterSide side, Vector3 start, Vector3 target, int gx, int gz, int owner, int slot, int type, int element)
{
    if (!animation)
        return;

    animation->side = side;
    animation->position = start;
    animation->target = target;
    animation->rotation = 0.0f;
    animation->active = true;

    animation->type = type;
    animation->element = element;
    animation->gx = gx;
    animation->gz = gz;

    animation->owner = owner;
    animation->slot = slot;

    animation->phase = MONSTER_ANIM_TRAJETORIA;
    animation->frame = 0;
    animation->frameTimer = 0;
}

bool AtualizarAnimacaoMonstro(MonsterAnimation *animation)
{
    if (!animation || !animation->active)
        return false;

    animation->frameTimer++;

    if (animation->phase == MONSTER_ANIM_TRAJETORIA)
    {
        float speed = 6.0f * GetFrameTime();

        animation->position.x += (animation->target.x - animation->position.x) * speed;
        animation->position.y += (animation->target.y - animation->position.y) * speed;
        animation->position.z += (animation->target.z - animation->position.z) * speed;
        animation->rotation += 720.0f * GetFrameTime();

        int element = animation->element;

        if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT)
            element = BAKUGAN_ELEMENT_FOGO;

        int total = rolagemAnimations[element].frameCount;

        if (animation->frameTimer >= 4)
        {
            animation->frameTimer = 0;
            animation->frame++;

            if (total <= 0)
                total = 1;

            if (animation->frame >= total)
                animation->frame = 0;
        }

        float dx = animation->target.x - animation->position.x;
        float dy = animation->target.y - animation->position.y;
        float dz = animation->target.z - animation->position.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        if (dist < 0.25f)
        {
            animation->position = animation->target;
            animation->phase = MONSTER_ANIM_BOLA_ELEMENTO;
            animation->frame = 0;
            animation->frameTimer = 0;
        }

        return false;
    }

    if (animation->phase == MONSTER_ANIM_BOLA_ELEMENTO)
    {
        animation->rotation += 360.0f * GetFrameTime();

        if (animation->frameTimer >= 60)
        {
            animation->phase = MONSTER_ANIM_TRANSFORMACAO;
            animation->frame = 0;
            animation->frameTimer = 0;
        }

        return false;
    }

    if (animation->phase == MONSTER_ANIM_TRANSFORMACAO)
    {
        if (animation->frameTimer >= 40)
        {
            animation->active = false;
            animation->rotation = 0.0f;
            return true;
        }

        return false;
    }

    return false;
}

void AtualizarTransformacaoMonstro(bool *transforming, int *transformTimer, int *transformStage)
{
    if (!transforming || !transformTimer || !transformStage || !*transforming)
        return;

    (*transformTimer)++;

    if (*transformTimer > 60)
        *transformStage = 1;

    if (*transformTimer > 120)
        *transformStage = 2;

    if (*transformTimer > 180)
        *transforming = false;
}

Texture2D SelecionarTexturaEstagioMonstro(Texture2D stage0, Texture2D stage1, Texture2D stage2, int transformStage)
{
    if (transformStage == 0)
        return stage0;

    if (transformStage == 1)
        return stage1;

    return stage2;
}

static bool SpriteFacesRightByDefault(int type)
{
    static const bool facesRight[BAKUGAN_TYPE_COUNT] = {
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        false,
        false,
        false,
        true,
        false
    };

    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        return false;

    return facesRight[type];
}

void DesenharMonstroBillboard(Camera3D camera, Texture2D texture, Vector3 position, Vector2 scale, int type, MonsterSide side)
{
    Rectangle source = {
        0,
        0,
        (float)texture.width,
        (float)texture.height
    };

    bool facesRight = SpriteFacesRightByDefault(type);

    bool flipHorizontal = (type < 0)
        ? (side == MONSTER_SIDE_LEFT)
        : ((side == MONSTER_SIDE_LEFT) ? !facesRight : facesRight);

    if (flipHorizontal)
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

void DesenharAnimacaoMonstroBillboard(Camera3D camera, const MonsterAnimation *animation, Vector2 scale)
{
    if (!animation)
        return;
    // Draw using JSON frames if available (preferred), otherwise spin the bola texture in screen space.
    int element = animation->element;
    if (element < 0 || element >= BAKUGAN_ELEMENT_COUNT) element = BAKUGAN_ELEMENT_FOGO;

    SpriteJsonAnimation *anim = &rolagemAnimations[element];
    Vector2 animScale = { scale.x * 1.12f, scale.y * 1.12f };

    if (animation->phase == MONSTER_ANIM_TRAJETORIA)
    {
        // prefer drawing the bola (rolling sphere) texture so launches are visible
        Texture2D bola = bolaElementoTextures[element];
        if (bola.id != 0)
        {
            Rectangle src = { 0, 0, (float)bola.width, (float)bola.height };
            DrawBillboardRec(
                camera,
                bola,
                src,
                animation->position,
                animScale,
                WHITE
            );
            return;
        }

        // if no bola texture, try JSON frames
        if (anim->texture.id != 0 && anim->frameCount > 0)
        {
            int frame = animation->frame;
            if (frame < 0 || frame >= anim->frameCount) frame = 0;
            Rectangle src = anim->frames[frame];
            DrawBillboardRec(camera, anim->texture, src, animation->position, animScale, WHITE);
            return;
        }

        // ultimate fallback: final bakugan texture
        Texture2D final = bakuganFinalTextures[animation->type >=0 ? animation->type : 0];
        Rectangle srcF = { 0, 0, (float)final.width, (float)final.height };
        DrawBillboardRec(camera, final, srcF, animation->position, animScale, WHITE);
        return;
    }

    if (animation->phase == MONSTER_ANIM_BOLA_ELEMENTO)
    {
        Texture2D bola = bolaElementoTextures[element];
        if (bola.id != 0)
        {
            Rectangle src = { 0, 0, (float)bola.width, (float)bola.height };
            DrawBillboardRec(
                camera,
                bola,
                src,
                animation->position,
                animScale,
                WHITE
            );
            return;
        }

        Texture2D final = bakuganFinalTextures[animation->type >=0 ? animation->type : 0];
        Rectangle srcF = { 0, 0, (float)final.width, (float)final.height };
        DrawBillboardRec(camera, final, srcF, animation->position, animScale, WHITE);
        return;
    }

    if (animation->phase == MONSTER_ANIM_TRANSFORMACAO)
    {
        Texture2D ttex = transformTextures[animation->type];
        if (ttex.id != 0)
        {
            Rectangle srcT = { 0, 0, (float)ttex.width, (float)ttex.height };
            DrawBillboardRec(camera, ttex, srcT, animation->position, animScale, WHITE);
            return;
        }

        Texture2D final = bakuganFinalTextures[animation->type >=0 ? animation->type : 0];
        Rectangle srcF2 = { 0, 0, (float)final.width, (float)final.height };
        DrawBillboardRec(camera, final, srcF2, animation->position, animScale, WHITE);
        return;
    }
}

MonsterPlacement MakeMonsterPlacement(int owner, int slot, int type, int element)
{
    MonsterPlacement monster;

    monster.owner = owner;
    monster.slot = slot;
    monster.type = type;
    monster.element = element;
    monster.power = 100;
    monster.side = MONSTER_SIDE_LEFT;

    return monster;
}

int BasePowerForType(int type)
{
    static const int power[BAKUGAN_TYPE_COUNT] = {
        200, // vento1
        120, // vento2
        210, // agua1
        130, // agua2
        310,  // terra1
        250, // terra2
        140, // fogo1
        260, // fogo2
        280,  // escuro1
        170, // escuro2
        230, // luz1
        180  // luz2
    };

    if (type < 0 || type >= BAKUGAN_TYPE_COUNT)
        return 1000;

    return power[type];
}

MonsterPlacement EmptyMonsterPlacement(void)
{
    MonsterPlacement monster;

    monster.owner = -1;
    monster.slot = -1;
    monster.power = 0;
    monster.type = -1;
    monster.element = -1;
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

int TipoDoMonstro(MonsterPlacement monster)
{
    return monster.type;
}

int ElementoDoMonstro(MonsterPlacement monster)
{
    return monster.element;
}

bool MonsterPlacementIsEmpty(MonsterPlacement monster)
{
    return monster.owner == -1;
}