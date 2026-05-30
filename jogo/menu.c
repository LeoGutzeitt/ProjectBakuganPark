#include "menu.h"
#include "raylib.h"

static EstadoMenu estadoAtual = MENU_INICIO;

static int jogadorAtual = 0;
static int escolhaAtual = 0;

static bool menuFinalizado = false;

static Texture2D bronzeCards[3];
static Texture2D prataCards[3];
static Texture2D ouroCards[3];

static Texture2D bakugans[12];

int bakugansEscolhidos[2][3];
static int quantidadeEscolhida = 0;

// =========================
// ESCOLHAS DOS JOGADORES
// =========================

int cartasBronzeEscolhidas[2];
int cartasPrataEscolhidas[2];
int cartasOuroEscolhidas[2];

bool MenuFinalizado()
{
    return menuFinalizado;
}

void AtualizarMenu()
{
    // =========================
    // MENU INICIAL
    // =========================

    if (estadoAtual == MENU_INICIO)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            estadoAtual = MENU_BRONZE;
            escolhaAtual = 0;
        }

        return;
    }

    // =========================
    // NAVEGAÇÃO CARTAS
    // =========================
    int maxEscolha =(estadoAtual == MENU_BAKUGAN) ? 11:2;

    if (estadoAtual != MENU_BAKUGAN)
    {
        if (IsKeyPressed(KEY_RIGHT))
        {
            escolhaAtual++;

            if (escolhaAtual > maxEscolha)
                escolhaAtual = 0;
        }

        if (IsKeyPressed(KEY_LEFT))
        {
            escolhaAtual--;

            if (escolhaAtual < 0)
                escolhaAtual = maxEscolha;
        }
    }

    // =========================
    // NAVEGAÇÃO BAKUGANS
    // =========================

    if (estadoAtual == MENU_BAKUGAN)
    {
        // DIREITA
        if (IsKeyPressed(KEY_RIGHT))
        {
            escolhaAtual++;

            if (escolhaAtual > 11)
                escolhaAtual = 0;
        }

        // ESQUERDA
        if (IsKeyPressed(KEY_LEFT))
        {
            escolhaAtual--;

            if (escolhaAtual < 0)
                escolhaAtual = 11;
        }

        // BAIXO
        if (IsKeyPressed(KEY_DOWN))
        {
            escolhaAtual += 4;

            if (escolhaAtual > 11)
                escolhaAtual -= 12;
        }

        // CIMA
        if (IsKeyPressed(KEY_UP))
        {
            escolhaAtual -= 4;

            if (escolhaAtual < 0)
                escolhaAtual += 12;
        }
    }

    // =========================
    // CONFIRMAR ESCOLHA
    // =========================

    if (IsKeyPressed(KEY_ENTER))
    {
        // =====================
        // BRONZE
        // =====================

        if (estadoAtual == MENU_BRONZE)
        {
            cartasBronzeEscolhidas[jogadorAtual] = escolhaAtual;

            estadoAtual = MENU_PRATA;
            escolhaAtual = 0;
        }

        // =====================
        // PRATA
        // =====================

        else if (estadoAtual == MENU_PRATA)
        {
            cartasPrataEscolhidas[jogadorAtual] = escolhaAtual;

            estadoAtual = MENU_OURO;
            escolhaAtual = 0;
        }

        // =====================
        // OURO
        // =====================

        else if (estadoAtual == MENU_OURO)
        {
            cartasOuroEscolhidas[jogadorAtual] = escolhaAtual;

            estadoAtual = MENU_BAKUGAN;

            escolhaAtual = 0;
            quantidadeEscolhida = 0;
        }

        // =====================
        // BAKUGANS
        // =====================

        else if (estadoAtual == MENU_BAKUGAN)
        {
            bakugansEscolhidos[jogadorAtual][quantidadeEscolhida] = escolhaAtual;

            quantidadeEscolhida++;

            // PLAYER ESCOLHEU 3
            if (quantidadeEscolhida >= 3)
            {
                // PLAYER 1 TERMINOU
                if (jogadorAtual == 0)
                {
                    jogadorAtual = 1;

                    estadoAtual = MENU_BRONZE;

                    escolhaAtual = 0;
                    quantidadeEscolhida = 0;
                }

                // PLAYER 2 TERMINOU
                else
                {
                    menuFinalizado = true;
                }
            }
        }
    }
}

void CarregarMenu()
{
    // =========================
    // CARTAS
    // =========================

    bronzeCards[0] = LoadTexture("img/card-bronze1-180x256px.png");
    bronzeCards[1] = LoadTexture("img/card-bronze2-180x256px.png");
    bronzeCards[2] = LoadTexture("img/card-bronze3-180x256px.png");

    prataCards[0] = LoadTexture("img/card-prata1-180x256px.png");
    prataCards[1] = LoadTexture("img/card-prata2-180x256px.png");
    prataCards[2] = LoadTexture("img/card-prata3-180x256px.png");

    ouroCards[0] = LoadTexture("img/card-ouro1-180x256px.png");
    ouroCards[1] = LoadTexture("img/card-ouro2-180x256px.png");
    ouroCards[2] = LoadTexture("img/card-ouro3-180x256px.png");

    // =========================
    // BAKUGANS
    // =========================

    bakugans[0] = LoadTexture("img/vento1.png");
    bakugans[1] = LoadTexture("img/vento2.png");

    bakugans[2] = LoadTexture("img/agua1.png");
    bakugans[3] = LoadTexture("img/agua2.png");

    bakugans[4] = LoadTexture("img/terra1.png");
    bakugans[5] = LoadTexture("img/terra2.png");

    bakugans[6] = LoadTexture("img/fogo1.png");
    bakugans[7] = LoadTexture("img/fogo2.png");

    bakugans[8] = LoadTexture("img/escuro1.png");
    bakugans[9] = LoadTexture("img/escuro2.png");

    bakugans[10] = LoadTexture("img/luz1.png");
    bakugans[11] = LoadTexture("img/luz2.png");
}

void DescarregarMenu()
{
    for (int i = 0; i < 3; i++)
    {
        UnloadTexture(bronzeCards[i]);
        UnloadTexture(prataCards[i]);
        UnloadTexture(ouroCards[i]);
    }

    for (int i = 0; i < 12; i++)
    {
        UnloadTexture(bakugans[i]);
    }
}

void ReiniciarMenu(void)
{
    estadoAtual = MENU_INICIO;
    jogadorAtual = 0;
    escolhaAtual = 0;
    quantidadeEscolhida = 0;
    menuFinalizado = false;

    for (int p = 0; p < 2; p++)
    {
        cartasBronzeEscolhidas[p] = 0;
        cartasPrataEscolhidas[p] = 0;
        cartasOuroEscolhidas[p] = 0;

        for (int i = 0; i < 3; i++)
        {
            bakugansEscolhidos[p][i] = 0;
        }
    }
}

void DesenharMenu()
{
    int screenWidth = GetScreenWidth();

    ClearBackground((Color){20, 20, 30, 255});

    // =========================
    // MENU INICIAL
    // =========================

    if (estadoAtual == MENU_INICIO)
    {
        DrawText(
            "BAKUGAN PARK",
            screenWidth / 2 - MeasureText("BAKUGAN PARK", 60) / 2,
            120,
            60,
            WHITE
        );

        DrawText(
            "PRESSIONE ENTER",
            screenWidth / 2 - MeasureText("PRESSIONE ENTER", 30) / 2,
            300,
            30,
            YELLOW
        );

        return;
    }

    // =========================
    // TITULO PLAYER
    // =========================

    DrawText(
        TextFormat("JOGADOR %d", jogadorAtual + 1),
        100,
        50,
        50,
        WHITE
    );

    // =========================
    // MENU BAKUGAN
    // =========================

    if (estadoAtual == MENU_BAKUGAN)
    {
        DrawText(
            TextFormat(
                "JOGADOR %d ESCOLHA SEUS BAKUGANS (%d/3)",
                jogadorAtual + 1,
                quantidadeEscolhida
            ),
            100,
            120,
            35,
            YELLOW
        );

        for (int i = 0; i < 12; i++)
        {
            int coluna = i % 4;
            int linha = i / 4;

            int x = 100 + (coluna * 200);
            int y = 180 + (linha * 160);

            DrawTextureEx(
                bakugans[i],
                (Vector2){x, y},
                0.0f,
                0.35f,
                WHITE
            );

            if (i == escolhaAtual)
            {
                Rectangle rec = {
                (float)(x - 5),
                (float)(y - 5),
                (float)(bakugans[i].width + 10),
                (float)(bakugans[i].height + 10)
            };

            DrawRectangleLinesEx(rec, 3.0f, RED);
            }
        }

        return;
    }

    // =========================
    // TEXTO DOS MENUS
    // =========================

    if (estadoAtual == MENU_BRONZE)
    {
        DrawText(
            "ESCOLHA SUA CARTA BRONZE",
            100,
            140,
            40,
            YELLOW
        );
    }

    if (estadoAtual == MENU_PRATA)
    {
        DrawText(
            "ESCOLHA SUA CARTA PRATA",
            100,
            140,
            40,
            LIGHTGRAY
        );
    }

    if (estadoAtual == MENU_OURO)
    {
        DrawText(
            "ESCOLHA SUA CARTA OURO",
            100,
            140,
            40,
            GOLD
        );
    }

    // =========================
    // DESENHAR CARTAS
    // =========================

    for (int i = 0; i < 3; i++)
    {
        int x = 180 + (i * 300);
        int y = 250;

        if (estadoAtual == MENU_BRONZE)
        {
            DrawTexture(bronzeCards[i], x, y, WHITE);
        }

        if (estadoAtual == MENU_PRATA)
        {
            DrawTexture(prataCards[i], x, y, WHITE);
        }

        if (estadoAtual == MENU_OURO)
        {
            DrawTexture(ouroCards[i], x, y, WHITE);
        }

        if (i == escolhaAtual)
        {
            DrawRectangleLinesEx(
                (Rectangle){
                    x - 5,
                    y - 5,
                    190,
                    266
                },
                5,
                RED
            );
        }
    }
}