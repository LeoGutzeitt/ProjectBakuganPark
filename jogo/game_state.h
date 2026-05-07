#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>

#define MAX_HAND_SLOTS 3

typedef struct {
    int owner;
    int slot;
    int power;
} MonsterPlacement;

typedef struct {
    int owner; // 0 or 1, -1 if none
    int slot;
} CardPlacement;

typedef struct {
    CardPlacement card;
    MonsterPlacement monsters[2];
    int monsterCount;
} TileEntity;

// Initialize game state for grid
void InitGameState(int gridX, int gridZ);
void FreeGameState(void);

// Player hand management (0 or 1)
void InitPlayerHands(void);
bool PlayerHasCardInHand(int player);
bool PlayerHasMonsterInHand(int player);
int PlayerCardSlotAvailable(int player);

// Place entity on map; returns true on success
bool PlaceCardAt(int gx, int gz, int owner, int slot);
bool PlaceMonsterAt(int gx, int gz, int owner, int slot);

// Resolve battle when a tile has 2 monsters.
// Returns true when the tile was resolved and cleared.
bool ResolveTileBattle(int gx, int gz, int *outWinnerOwner);
void ClearTile(int gx, int gz);

// Query
int CountPlayerCardsOnMap(int player);
TileEntity GetTileAt(int gx, int gz);
bool TileHasCard(int gx, int gz);
int GetTileMonsterCount(int gx, int gz);

int GetGridSizeX(void);
int GetGridSizeZ(void);

// Remove from hand when placed
void RemovePlayerCardFromHand(int player, int slot);
void RemovePlayerMonsterFromHand(int player, int slot);

// Copy current hands into provided arrays (must be size [2][3])
void GetPlayerHands(int outCards[2][3], int outMonsters[2][3]);

#endif // GAME_STATE_H
