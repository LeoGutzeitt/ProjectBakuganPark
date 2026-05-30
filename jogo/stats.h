#ifndef STATS_H
#define STATS_H

#include <stdbool.h>

#include "game_state.h"

#define MAX_REGISTROS_BATALHA 16

typedef struct {
    char titulo[64];
    char linhaCarta[80];
    char linhaBatalha[120];
    char linhaVencedor[80];
} RegistroBatalha;

typedef struct {
    int jogadorVencedor;
    int quantidadeBatalhas;
    RegistroBatalha registros[MAX_REGISTROS_BATALHA];
} EstatisticasPartida;

void ReiniciarProgressoPartida(EstatisticasPartida *stats, int *vencedorJogador, int *pontuacaoJogador1, int *pontuacaoJogador2);
void MontarRegistroBatalha(RegistroBatalha *record, int indiceBatalha, int gx, int gz, TileEntity battleTile, int vencedorDono);
void SalvarEstatisticasPartidaEmArquivo(const EstatisticasPartida *stats, int pontuacaoJogador1, int pontuacaoJogador2);

void DesenharTelaEstatisticas(int screenWidth, int screenHeight, const EstatisticasPartida *estatisticas, int player1Score, int player2Score);
#endif // STATS_H