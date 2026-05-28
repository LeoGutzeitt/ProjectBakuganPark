#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    TELA_MENU_PRINCIPAL = 0,
    TELA_PREPARACAO_DECK = 1,
    TELA_BATALHA = 2,
    TELA_VITORIA = 3,
    TELA_ESTATISTICAS = 4
} TelaAplicacao;

typedef struct {
    int activePlayer;
    int selectedRow;
    int selectedSlot;
    int selectedChoice;
    int cardTypes[2][3];
    int monsterTypes[2][3];
    int monsterElements[2][3];
    int awaitingElementSlot; // -1 when not awaiting
} EstadoPreparacaoDeck;

void InicializarEstadoPreparacaoDeck(EstadoPreparacaoDeck *state);
bool AtualizarEstadoPreparacaoDeck(EstadoPreparacaoDeck *state);
void DesenharTelaMenuPrincipal(int screenWidth, int screenHeight);
void DesenharTelaPreparacaoDeck(int screenWidth, int screenHeight, const EstadoPreparacaoDeck *state);
void DesenharTelaVitoria(int screenWidth, int screenHeight, int vencedorJogador, int pontuacaoJogador1, int pontuacaoJogador2);

#endif // MENU_H