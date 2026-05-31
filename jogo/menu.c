#include "menu.h"
#include "raylib.h"
#include "card.h"
#include "monster.h"

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

static bool BakuganJaEscolhido(int jogador, int escolha)
{
    for (int i = 0; i < quantidadeEscolhida; i++)
    {
        if (bakugansEscolhidos[jogador][i] == escolha)
            return true;
    }

    return false;
}

static const char *CardStatLabel(int index)
{
    static const char *labels[6] = { "Fogo", "Agua", "Terra", "Luz", "Escuridao", "Vento" };

    if (index < 0 || index >= 6) return "?";
    return labels[index];
}

static int CardStatToElement(int index)
{
    static const int elements[6] = {
        BAKUGAN_ELEMENT_FOGO,
        BAKUGAN_ELEMENT_AGUA,
        BAKUGAN_ELEMENT_TERRA,
        BAKUGAN_ELEMENT_LUZ,
        BAKUGAN_ELEMENT_ESCURO,
        BAKUGAN_ELEMENT_VENTO
    };

    if (index < 0 || index >= 6) return BAKUGAN_ELEMENT_FOGO;
    return elements[index];
}

static void DrawCardStatsPanel(int x, int y, int w, int h, int slot, int cardIndex, Texture2D texture)
{
    DrawRectangle(x, y, w, h, (Color){ 34, 43, 62, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, (float)w, (float)h }, 2.0f, (Color){ 255, 221, 117, 255 });
    DrawText("PONTOS DA CARTA", x + 14, y + 12, 18, YELLOW);
    DrawText(TextFormat("%s C%d", slot == 0 ? "Bronze" : (slot == 1 ? "Prata" : "Ouro"), cardIndex + 1), x + 14, y + 38, 14, RAYWHITE);

    DrawTextureEx(texture, (Vector2){ (float)x + 14.0f, (float)y + 66.0f }, 0.0f, 0.42f, WHITE);

    for (int row = 0; row < 3; row++)
    {
        int leftIndex = row * 2;
        int rightIndex = leftIndex + 1;
        int leftValue = CardBonusForPortalCard(slot, cardIndex, CardStatToElement(leftIndex));
        int rightValue = CardBonusForPortalCard(slot, cardIndex, CardStatToElement(rightIndex));

        DrawText(TextFormat("%s %d", CardStatLabel(leftIndex), leftValue), x + 120, y + 70 + row * 22, 12, SKYBLUE);
        DrawText(TextFormat("%s %d", CardStatLabel(rightIndex), rightValue), x + 238, y + 70 + row * 22, 12, SKYBLUE);
    }
}

static void DrawBakuganBasePanel(int x, int y, int w, int h, int index)
{
    DrawRectangle(x, y, w, h, (Color){ 34, 43, 62, 255 });
    DrawRectangleLinesEx((Rectangle){ (float)x, (float)y, (float)w, (float)h }, 2.0f, (Color){ 255, 221, 117, 255 });
    DrawText("G POWER BASE", x + 14, y + 12, 18, YELLOW);
    DrawText(BakuganTypeName(index), x + 14, y + 40, 22, WHITE);
    DrawText(TextFormat("Base %d", BasePowerForType(index)), x + 14, y + 74, 28, GOLD);
}

static Texture2D ObterBakuganTextureMenu(int index)
{
    if (index < 0 || index >= 12)
        return bakugans[0];

    return bakugans[index];
}

Texture2D ObterCartaTexturePorSlot(int slot, int escolha)
{
    if (escolha < 0 || escolha >= 3)
        escolha = 0;

    if (slot <= 0)
        return bronzeCards[escolha];

    if (slot == 1)
        return prataCards[escolha];

    return ouroCards[escolha];
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
        Rectangle previewPanel = { 840.0f, 150.0f, 920.0f, 620.0f };

        DrawRectangleRec(previewPanel, (Color){ 34, 43, 62, 255 });
        DrawRectangleLinesEx(previewPanel, 2.0f, (Color){ 255, 221, 117, 255 });

        DrawText(TextFormat("JOGADOR %d ESCOLHE BAKUGANS (%d/3)", jogadorAtual + 1, quantidadeEscolhida), 100, 120, 30, YELLOW);

        DrawText("PREVIA DO BAKUGAN", (int)previewPanel.x + 28, (int)previewPanel.y + 18, 24, WHITE);

        Texture2D previewTexture = ObterBakuganTextureMenu(escolhaAtual);
        DrawTextureEx(
            previewTexture,
            (Vector2){ previewPanel.x + 34.0f, previewPanel.y + 84.0f },
            0.0f,
            1.35f,
            WHITE
        );

        DrawRectangleLinesEx(
            (Rectangle){ previewPanel.x + 30.0f, previewPanel.y + 80.0f, (float)(previewTexture.width * 1.35f) + 8.0f, (float)(previewTexture.height * 1.35f) + 8.0f },
            2.0f,
            (Color){ 255, 255, 255, 70 }
        );

        DrawText(BakuganTypeName(escolhaAtual), (int)previewPanel.x + 360, (int)previewPanel.y + 98, 30, WHITE);
        DrawText(TextFormat("Atual: %d/3", quantidadeEscolhida + 1), (int)previewPanel.x + 360, (int)previewPanel.y + 146, 20, SKYBLUE);
        DrawText("ENTER confirma", (int)previewPanel.x + 360, (int)previewPanel.y + 182, 18, RAYWHITE);
        DrawText(TextFormat("Base G: %d", BasePowerForType(escolhaAtual)), (int)previewPanel.x + 360, (int)previewPanel.y + 214, 26, GOLD);

        DrawText("Escolhidos", (int)previewPanel.x + 360, (int)previewPanel.y + 246, 20, YELLOW);

        for (int i = 0; i < 3; i++)
        {
            int chosenIndex = bakugansEscolhidos[jogadorAtual][i];
            int slotX = (int)previewPanel.x + 360 + i * 150;
            int slotY = (int)previewPanel.y + 286;

            DrawRectangle(slotX, slotY, 124, 124, (Color){ 255, 255, 255, 20 });
            DrawRectangleLines(slotX, slotY, 124, 124, i < quantidadeEscolhida ? GREEN : (Color){ 255, 255, 255, 60 });

            if (i < quantidadeEscolhida)
            {
                Texture2D chosenTexture = ObterBakuganTextureMenu(chosenIndex);
                DrawTextureEx(chosenTexture, (Vector2){ (float)slotX + 10.0f, (float)slotY + 10.0f }, 0.0f, 0.45f, WHITE);
                DrawText(TextFormat("%d", i + 1), slotX + 86, slotY + 8, 18, YELLOW);
            }
            else
            {
                DrawText(TextFormat("%d", i + 1), slotX + 54, slotY + 52, 18, DARKGRAY);
            }
        }

        DrawText("Borda verde = escolhido", (int)previewPanel.x + 360, (int)previewPanel.y + 448, 16, (Color){ 180, 235, 180, 255 });
        DrawBakuganBasePanel((int)previewPanel.x + 360, (int)previewPanel.y + 500, 260, 90, escolhaAtual);

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

            bool alreadyChosen = BakuganJaEscolhido(jogadorAtual, i);

            if (alreadyChosen)
            {
                DrawRectangle(x, y, (int)(bakugans[i].width * 0.35f), (int)(bakugans[i].height * 0.35f), (Color){ 70, 140, 70, 70 });
                DrawRectangleLines(x - 2, y - 2, (int)(bakugans[i].width * 0.35f) + 4, (int)(bakugans[i].height * 0.35f) + 4, GREEN);
            }

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

        DrawCardStatsPanel(1120, 170, 540, 300, 0, escolhaAtual, bronzeCards[escolhaAtual]);
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

        DrawCardStatsPanel(1120, 170, 540, 300, 1, escolhaAtual, prataCards[escolhaAtual]);
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

        DrawCardStatsPanel(1120, 170, 540, 300, 2, escolhaAtual, ouroCards[escolhaAtual]);
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