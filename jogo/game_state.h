#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include "card.h"
#include "monster.h"

#define MAX_HAND_SLOTS 3

typedef struct {
    CardPlacement card;
    MonsterPlacement monsters[2];
    int monsterCount;
} TileEntity;

// Inicializa o estado do jogo para o grid
void InicializarEstadoJogo(int gridX, int gridZ);
void LiberarEstadoJogo(void);

// Gerenciamento da mão dos jogadores (0 ou 1)
void InicializarMaoJogadores(void);
bool JogadorTemCartaNaMao(int player);
bool JogadorTemMonstroNaMao(int player);
int SlotCartaDisponivelJogador(int player);

// Coloca entidades no mapa; retorna true em caso de sucesso
bool PodeColocarCartaEm(int gx, int gz);
bool ColocarCartaEm(int gx, int gz, int owner, int slot, int type);
bool ColocarMonstroEm(int gx, int gz, int owner, int slot, int type, int element, MonsterSide side);
bool AtualizarPoderMonstroNoTile(int gx, int gz, int owner, int slot, int power);

// Resolve a batalha quando um tile tem 2 monstros.
// Retorna true quando o tile foi resolvido e limpo.
bool ResolverBatalhaNoTile(int gx, int gz, int *outWinnerOwner);
// Determina o vencedor de uma batalha sem limpar o tile (útil para animações antes
// de confirmar/limpar). Retorna true se havia batalha e escreve o dono vencedor em
// outWinnerOwner (se não for NULL).
bool VerificarVencedorBatalhaNoTile(int gx, int gz, int *outWinnerOwner);
void LimparTile(int gx, int gz);

// Consultas
int ContarCartasJogadorNoMapa(int player);
int ContarCartasNoMapa(void);
TileEntity ObterTileEm(int gx, int gz);
bool TileTemCarta(int gx, int gz);
int ContarMonstrosNoTile(int gx, int gz);
bool RemoverMonstroTilePorDonoSlot(int gx, int gz, int owner, int slot);

int ObterTamanhoGridX(void);
int ObterTamanhoGridZ(void);

// Remove da mão quando colocado
void RemoverCartaJogadorDaMao(int player, int slot);
void RemoverMonstroJogadorDaMao(int player, int slot);

// Copia as mãos atuais para os vetores informados (devem ter tamanho [2][3])
void ObterMaosJogadores(int outCards[2][3], int outMonsters[2][3]);

#endif // fim de GAME_STATE_H
