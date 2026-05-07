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
void InitGameState(int gridX, int gridZ);
void FreeGameState(void);

// Gerenciamento da mão dos jogadores (0 ou 1)
void InitPlayerHands(void);
bool PlayerHasCardInHand(int player);
bool PlayerHasMonsterInHand(int player);
int PlayerCardSlotAvailable(int player);

// Coloca entidades no mapa; retorna true em caso de sucesso
bool CanPlaceCardAt(int gx, int gz);
bool PlaceCardAt(int gx, int gz, int owner, int slot);
bool PlaceMonsterAt(int gx, int gz, int owner, int slot);

// Resolve a batalha quando um tile tem 2 monstros.
// Retorna true quando o tile foi resolvido e limpo.
bool ResolveTileBattle(int gx, int gz, int *outWinnerOwner);
void ClearTile(int gx, int gz);

// Consultas
int CountPlayerCardsOnMap(int player);
int CountCardsOnMap(void);
TileEntity GetTileAt(int gx, int gz);
bool TileHasCard(int gx, int gz);
int GetTileMonsterCount(int gx, int gz);

int GetGridSizeX(void);
int GetGridSizeZ(void);

// Remove da mão quando colocado
void RemovePlayerCardFromHand(int player, int slot);
void RemovePlayerMonsterFromHand(int player, int slot);

// Copia as mãos atuais para os vetores informados (devem ter tamanho [2][3])
void GetPlayerHands(int outCards[2][3], int outMonsters[2][3]);

#endif // fim de GAME_STATE_H
