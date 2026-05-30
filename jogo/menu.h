#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum
{
    MENU_INICIO,
    MENU_BRONZE,
    MENU_PRATA,
    MENU_OURO,
    MENU_BAKUGAN,
    MENU_FINALIZADO
} EstadoMenu;

void AtualizarMenu();
void DesenharMenu();
void CarregarMenu();
void DescarregarMenu();
void ReiniciarMenu();

bool MenuFinalizado();

extern int cartasBronzeEscolhidas[2];
extern int cartasPrataEscolhidas[2];
extern int cartasOuroEscolhidas[2];

extern int bakugansEscolhidos[2][3];

#endif