CC = gcc
CFLAGS = -g -O2
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

SRCS = jogo/main.c jogo/battle_map.c jogo/card.c jogo/monster.c jogo/game_state.c jogo/ui.c
TARGET = jogo/game

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
