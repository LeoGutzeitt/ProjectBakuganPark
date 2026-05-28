#include "stats.h"

#include <stdio.h>
#include <string.h>

#include "card.h"
#include "monster.h"
#include "raylib.h"

static void DrawFooterHintBar(int screenWidth, int screenHeight, const char *leftHint, const char *rightHint)
{
    DrawRectangle(0, screenHeight - 92, screenWidth, 92, (Color){20, 26, 40, 255});
    DrawRectangle(0, screenHeight - 92, screenWidth, 4, (Color){255, 255, 255, 60});
    DrawText(leftHint, 34, screenHeight - 62, 20, RAYWHITE);
    DrawText(rightHint, screenWidth - 34 - MeasureText(rightHint, 20), screenHeight - 62, 20, SKYBLUE);
}

void ReiniciarProgressoPartida(EstatisticasPartida *stats, int *vencedorJogador, int *pontuacaoJogador1, int *pontuacaoJogador2)
{
    if (stats) {
        memset(stats, 0, sizeof(*stats));
        stats->jogadorVencedor = -1;
    }

    if (vencedorJogador) *vencedorJogador = -1;
    if (pontuacaoJogador1) *pontuacaoJogador1 = 0;
    if (pontuacaoJogador2) *pontuacaoJogador2 = 0;
}

void MontarRegistroBatalha(RegistroBatalha *record, int indiceBatalha, int gx, int gz, TileEntity battleTile, int vencedorDono)
{
    if (!record) return;

    memset(record, 0, sizeof(*record));

    MonsterPlacement m0 = battleTile.monsters[0];
    MonsterPlacement m1 = battleTile.monsters[1];

    snprintf(record->titulo, sizeof(record->titulo), "Batalha %d | Grid %d,%d", indiceBatalha + 1, gx + 1, gz + 1);
    snprintf(record->linhaCarta, sizeof(record->linhaCarta), "Carta usada: %s (P%d)", CardTypeName(battleTile.card.type), battleTile.card.owner + 1);
    snprintf(
        record->linhaBatalha,
        sizeof(record->linhaBatalha),
        "Bakugans: P%d %s/%s vs P%d %s/%s",
        m0.owner + 1,
        BakuganTypeName(m0.type),
        BakuganElementName(m0.element),
        m1.owner + 1,
        BakuganTypeName(m1.type),
        BakuganElementName(m1.element)
    );

    const MonsterPlacement *vencedorMonstro = &m0;

    if (m1.owner == vencedorDono && m0.owner != vencedorDono) {
        vencedorMonstro = &m1;
    } else if (m0.owner == vencedorDono && m1.owner == vencedorDono && m1.power > m0.power) {
        vencedorMonstro = &m1;
    }

    snprintf(
        record->linhaVencedor,
        sizeof(record->linhaVencedor),
        "Vencedor: P%d %s/%s",
        vencedorMonstro->owner + 1,
        BakuganTypeName(vencedorMonstro->type),
        BakuganElementName(vencedorMonstro->element)
    );
}

void SalvarEstatisticasPartidaEmArquivo(const EstatisticasPartida *stats, int pontuacaoJogador1, int pontuacaoJogador2)
{
    if (!stats) return;

    FILE *file = fopen("output/estatisticas_partida.txt", "w");
    if (!file) return;

    fprintf(file, "Estatisticas da partida\n");
    fprintf(file, "Placar final: P1 %d - P2 %d\n", pontuacaoJogador1, pontuacaoJogador2);
    if (stats->jogadorVencedor >= 0) {
        fprintf(file, "Vencedor da partida: P%d\n", stats->jogadorVencedor + 1);
    }
    fprintf(file, "Batalhas registradas: %d\n\n", stats->quantidadeBatalhas);

    for (int i = 0; i < stats->quantidadeBatalhas; i++) {
        const RegistroBatalha *batalha = &stats->registros[i];
        fprintf(file, "%s\n", batalha->titulo);
        fprintf(file, "%s\n", batalha->linhaCarta);
        fprintf(file, "%s\n", batalha->linhaBatalha);
        fprintf(file, "%s\n\n", batalha->linhaVencedor);
    }

    fclose(file);
}

void DesenharTelaEstatisticas(int screenWidth, int screenHeight, const EstatisticasPartida *stats, int pontuacaoJogador1, int pontuacaoJogador2)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){18, 24, 36, 255});
    DrawCircleGradient(screenWidth / 2, 240, 520.0f, (Color){90, 130, 210, 150}, (Color){18, 24, 36, 0});

    DrawText("ESTATISTICAS", 40, 32, 56, RAYWHITE);
    DrawText("Resumo das batalhas da partida", 44, 92, 22, SKYBLUE);

    DrawRectangle(40, 140, screenWidth - 80, 100, (Color){28, 36, 54, 245});
    DrawRectangleLines(40, 140, screenWidth - 80, 100, (Color){255, 255, 255, 70});
    DrawText(TextFormat("Batalhas registradas: %d", stats ? stats->quantidadeBatalhas : 0), 64, 170, 24, RAYWHITE);
    DrawText(TextFormat("Placar final: P1 %d  -  P2 %d", pontuacaoJogador1, pontuacaoJogador2), 64, 204, 24, SKYBLUE);

    int visibleCount = 0;
    int startIndex = 0;

    if (stats && stats->quantidadeBatalhas > 0) {
        visibleCount = stats->quantidadeBatalhas;
        if (visibleCount > 4) visibleCount = 4;
        startIndex = stats->quantidadeBatalhas - visibleCount;
    }

    int cardX = 40;
    int cardY = 270;
    int cardW = screenWidth - 80;
    int cardH = 138;
    int gap = 16;

    if (!stats || stats->quantidadeBatalhas == 0) {
        DrawRectangle(cardX, cardY, cardW, cardH, (Color){28, 36, 54, 220});
        DrawRectangleLines(cardX, cardY, cardW, cardH, (Color){255, 255, 255, 60});
        DrawText("Nenhuma batalha foi registrada", cardX + 28, cardY + 48, 26, RAYWHITE);
        DrawText("Volte ao jogo para preencher este painel", cardX + 28, cardY + 84, 18, SKYBLUE);
    } else {
        for (int i = 0; i < visibleCount; i++) {
            const RegistroBatalha *batalha = &stats->registros[startIndex + i];
            int y = cardY + i * (cardH + gap);

            DrawRectangle(cardX, y, cardW, cardH, (Color){28, 36, 54, 220});
            DrawRectangleLines(cardX, y, cardW, cardH, (Color){255, 255, 255, 60});
            DrawRectangle(cardX, y, 14, cardH, (i % 2 == 0) ? BLUE : RED);

            DrawText(batalha->titulo, cardX + 28, y + 18, 22, RAYWHITE);
            DrawText(batalha->linhaCarta, cardX + 28, y + 50, 20, YELLOW);
            DrawText(batalha->linhaBatalha, cardX + 28, y + 78, 18, RAYWHITE);
            DrawText(batalha->linhaVencedor, cardX + 28, y + 108, 20, (i % 2 == 0) ? SKYBLUE : ORANGE);
        }
    }

    DrawFooterHintBar(screenWidth, screenHeight, "ESC volta para a vitoria", "H / ENTER vai para a tela inicial");
}