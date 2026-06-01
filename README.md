# ProjectBakuganPark

Projeto desenvolvido em C utilizando a biblioteca **raylib**, como trabalho da disciplina de **Programação Imperativa e Funcional (PIF)**.

A proposta do jogo é criar uma versão inspirada em batalhas de Bakugan, com escolha de cartas, seleção de personagens, posicionamento no tabuleiro e resolução de batalhas com base em atributos e bônus.

## Sobre o jogo

O **ProjectBakuganPark** é um jogo em que dois jogadores escolhem cartas e Bakugans antes da partida começar. Depois disso, os jogadores alternam turnos para posicionar cartas no tabuleiro e lançar seus Bakugans sobre elas.

Quando dois Bakugans ocupam a mesma carta, ocorre uma batalha. O resultado leva em consideração o poder base do Bakugan e os bônus oferecidos pela carta portal.

## Funcionalidades

* Menu inicial com imagem de introdução
* Seleção de cartas por jogador
* Seleção de Bakugans por jogador
* Tabuleiro em grade
* Sistema de turnos
* Posicionamento de cartas no mapa
* Lançamento de Bakugans
* Animação de rolagem e transformação dos Bakugans
* Sistema de colisão entre Bakugans
* Deslocamento visual do Bakugan atingido
* Cálculo de poder com base no Bakugan e na carta
* Sistema de batalha
* Tela de vitória
* Tela de estatísticas da partida

## Tecnologias utilizadas

* Linguagem C
* Biblioteca raylib
* MinGW / GCC
* Git e GitHub

## Estrutura do projeto

```txt
ProjectBakuganPark/
│
└── jogo/
    ├── main.c
    ├── menu.c
    ├── menu.h
    ├── monster.c
    ├── monster.h
    ├── card.c
    ├── card.h
    ├── game_state.c
    ├── game_state.h
    ├── battle_map.c
    ├── battle_map.h
    ├── stats.c
    ├── stats.h
    ├── ui.c
    ├── ui.h
    │
    └── img/
        ├── cartas
        ├── bakugans
        ├── animações
        └── fundos
```

## Como compilar

Para compilar o projeto, é necessário ter o **GCC** e a **raylib** instalados.

No terminal, dentro da pasta `jogo`, execute:

```bash
gcc main.c menu.c battle_map.c game_state.c stats.c ui.c monster.c card.c -o jogo -lraylib -lopengl32 -lgdi32 -lwinmm
```

Também é possível compilar com:

```bash
gcc *.c -o jogo -lraylib -lopengl32 -lgdi32 -lwinmm
```

Atenção: use `gcc *.c` apenas se a pasta não tiver arquivos `.c` duplicados, como cópias antigas da `main.c`, pois isso pode gerar erro de múltiplas funções `main`.

## Como executar

Depois de compilar, execute:

```bash
./jogo
```

## Controles

### Menu

* `ENTER` - confirmar escolha
* `Setas` - navegar entre cartas ou Bakugans

### Jogo

* `W`, `A`, `S`, `D` - mover seleção no tabuleiro
* `ENTER` - marcar tile
* `C` - escolher carta
* `M` - escolher Bakugan
* `ESPAÇO` - confirmar lançamento do Bakugan
* `BACKSPACE` - cancelar ação
* `F` - ativação do jogador 1 na batalha
* `L` - ativação do jogador 2 na batalha
* `T` - alternar jogador manualmente

## Mecânica da batalha

Cada Bakugan possui um poder base. Durante a batalha, esse poder pode ser aumentado de acordo com a carta onde ele caiu.

O cálculo geral funciona da seguinte forma:

```txt
Poder final = Poder base do Bakugan + Bônus da carta
```

O Bakugan com maior poder vence a batalha. Quando a batalha é finalizada, o vencedor recebe ponto.

## Objetivo

O objetivo é vencer as batalhas e alcançar a pontuação necessária para ganhar a partida.

## Observações

Este projeto foi desenvolvido com fins acadêmicos, buscando aplicar conceitos estudados na disciplina de PIF, como:

* Organização modular do código
* Uso de structs
* Manipulação de estados do jogo
* Controle de fluxo
* Funções
* Vetores e matrizes
* Renderização gráfica com raylib
* Separação de responsabilidades entre arquivos `.c` e `.h`

## Status do projeto

Projeto em desenvolvimento.

Algumas funcionalidades ainda podem ser ajustadas ou melhoradas, como balanceamento dos poderes, refinamento das animações, melhorias visuais e ajustes na interface.

## Autor

Projeto desenvolvido por estudantes da disciplina de **Programação Imperativa e Funcional (PIF)**.

##MAKEFILE

```txt
CC = gcc
CFLAGS = -g -O2
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

SRCS = jogo/main.c jogo/battle_map.c jogo/card.c jogo/monster.c jogo/game_state.c jogo/ui.c jogo/menu.c jogo/stats.c
TARGET = jogo/game

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

run: $(TARGET)
	cd $(dir $(TARGET)) && ./$(notdir $(TARGET))

clean:
	rm -f $(TARGET)

```
