#include "game_state.h"
#include <stdlib.h>

static int g_gridX = 0;
static int g_gridZ = 0;
static TileEntity *g_tiles = NULL;

static int g_playerCards[2][MAX_HAND_SLOTS];
static int g_playerMonsters[2][MAX_HAND_SLOTS];

static TileEntity *GetTilePtr(int gx, int gz)
{
    if (!g_tiles) return NULL;
    if (gx < 0 || gz < 0 || gx >= g_gridX || gz >= g_gridZ) return NULL;
    return &g_tiles[gz * g_gridX + gx];
}

void InitGameState(int gridX, int gridZ)
{
    g_gridX = gridX;
    g_gridZ = gridZ;
    g_tiles = (TileEntity*)malloc(sizeof(TileEntity) * gridX * gridZ);
    for (int i = 0; i < gridX * gridZ; i++) {
        g_tiles[i].card = CartaVazia();
        g_tiles[i].monsterCount = 0;
        for (int m = 0; m < 2; m++) {
            g_tiles[i].monsters[m] = EmptyMonsterPlacement();
        }
    }
    InitPlayerHands();
}

void FreeGameState(void)
{
    if (g_tiles) free(g_tiles);
    g_tiles = NULL;
}

void InitPlayerHands(void)
{
    for (int p = 0; p < 2; p++) {
        for (int s = 0; s < MAX_HAND_SLOTS; s++) {
            g_playerCards[p][s] = 1;
            g_playerMonsters[p][s] = 1;
        }
    }
}

bool PlayerHasCardInHand(int player)
{
    for (int s = 0; s < MAX_HAND_SLOTS; s++) if (g_playerCards[player][s]) return true;
    return false;
}

bool PlayerHasMonsterInHand(int player)
{
    for (int s = 0; s < MAX_HAND_SLOTS; s++) if (g_playerMonsters[player][s]) return true;
    return false;
}

int PlayerCardSlotAvailable(int player)
{
    for (int s = 0; s < MAX_HAND_SLOTS; s++) if (g_playerCards[player][s]) return s;
    return -1;
}

bool CanPlaceCardAt(int gx, int gz)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return false;
    if (tile->card.owner != -1) return false;

    // Limite global de cartas no tabuleiro.
    if (CountCardsOnMap() >= 4) return false;

    // Primeira carta da partida pode ser colocada em qualquer tile vazio.
    bool hasAnyCard = false;
    for (int z = 0; z < g_gridZ; z++) {
        for (int x = 0; x < g_gridX; x++) {
            if (g_tiles[z * g_gridX + x].card.owner != -1) {
                hasAnyCard = true;
                break;
            }
        }
        if (hasAnyCard) break;
    }
    if (!hasAnyCard) return true;

    // Depois da primeira, só pode em tiles vizinhos (inclui diagonal).
    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dz == 0) continue;
            int nx = gx + dx;
            int nz = gz + dz;
            TileEntity *neighbor = GetTilePtr(nx, nz);
            if (neighbor && neighbor->card.owner != -1) return true;
        }
    }

    return false;
}

bool PlaceCardAt(int gx, int gz, int owner, int slot)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return false;
    if (!CanPlaceCardAt(gx, gz)) return false;
    tile->card = CriarCarta(owner, slot);
    return true;
}

bool PlaceMonsterAt(int gx, int gz, int owner, int slot)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return false;
    if (tile->card.owner == -1) return false;
    if (tile->monsterCount >= 2) return false;
    tile->monsters[tile->monsterCount] = MakeMonsterPlacement(owner, slot);
    tile->monsterCount++;
    return true;
}

void ClearTile(int gx, int gz)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return;
    tile->card = CartaVazia();
    tile->monsterCount = 0;
    for (int m = 0; m < 2; m++) {
        tile->monsters[m] = EmptyMonsterPlacement();
    }
}

bool ResolveTileBattle(int gx, int gz, int *outWinnerOwner)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return false;
    if (tile->monsterCount < 2) return false;

    int winnerOwner = tile->card.owner;
    MonsterPlacement m0 = tile->monsters[0];
    MonsterPlacement m1 = tile->monsters[1];

    if (m0.owner == m1.owner) {
        winnerOwner = m0.owner;
    } else if (m0.power > m1.power) {
        winnerOwner = m0.owner;
    } else if (m1.power > m0.power) {
        winnerOwner = m1.owner;
    } else {
        winnerOwner = tile->card.owner;
    }

    if (outWinnerOwner) *outWinnerOwner = winnerOwner;
    ClearTile(gx, gz);
    return true;
}

int CountPlayerCardsOnMap(int player)
{
    int c = 0;
    for (int z = 0; z < g_gridZ; z++) {
        for (int x = 0; x < g_gridX; x++) {
            TileEntity te = g_tiles[z * g_gridX + x];
            if (te.card.owner == player) c++;
        }
    }
    return c;
}

int CountCardsOnMap(void)
{
    int c = 0;
    for (int z = 0; z < g_gridZ; z++) {
        for (int x = 0; x < g_gridX; x++) {
            TileEntity te = g_tiles[z * g_gridX + x];
            if (te.card.owner != -1) c++;
        }
    }
    return c;
}

TileEntity GetTileAt(int gx, int gz)
{
    TileEntity empty;
    empty.card = CartaVazia();
    empty.monsterCount = 0;
    for (int m = 0; m < 2; m++) {
        empty.monsters[m] = EmptyMonsterPlacement();
    }

    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return empty;
    return *tile;
}

bool TileHasCard(int gx, int gz)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return false;
    return tile->card.owner != -1;
}

int GetTileMonsterCount(int gx, int gz)
{
    TileEntity *tile = GetTilePtr(gx, gz);
    if (!tile) return 0;
    return tile->monsterCount;
}

int GetGridSizeX(void) { return g_gridX; }
int GetGridSizeZ(void) { return g_gridZ; }

void RemovePlayerCardFromHand(int player, int slot)
{
    if (player < 0 || player > 1) return;
    if (slot < 0 || slot >= MAX_HAND_SLOTS) return;
    g_playerCards[player][slot] = 0;
}

void RemovePlayerMonsterFromHand(int player, int slot)
{
    if (player < 0 || player > 1) return;
    if (slot < 0 || slot >= MAX_HAND_SLOTS) return;
    g_playerMonsters[player][slot] = 0;

    // Recarrega os monstros quando o jogador esgota os 3 slots.
    bool anyMonsterLeft = false;
    for (int s = 0; s < MAX_HAND_SLOTS; s++) {
        if (g_playerMonsters[player][s]) {
            anyMonsterLeft = true;
            break;
        }
    }

    if (!anyMonsterLeft) {
        for (int s = 0; s < MAX_HAND_SLOTS; s++) {
            g_playerMonsters[player][s] = 1;
        }
    }
}

void GetPlayerHands(int outCards[2][3], int outMonsters[2][3])
{
    if (!outCards || !outMonsters) return;
    for (int p = 0; p < 2; p++) {
        for (int s = 0; s < MAX_HAND_SLOTS; s++) {
            outCards[p][s] = g_playerCards[p][s];
            outMonsters[p][s] = g_playerMonsters[p][s];
        }
    }
}
