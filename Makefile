CC = cc
BIN = qik

CFILES = src/config.c \
	src/control.c \
	src/gfx/graphics.c \
	src/gfx/image.c \
	src/global.c \
	src/main.c \
	src/map.c \
	src/render.c

CFLAGS = -Iinclude -Wall -Wextra -lm -lSDL2 -lconfig -O2
DEBUGS = -O0 -g

all: $(BIN)

debug: CFLAGS += $(DEBUGS)
debug: all

$(BIN): $(CFILES)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	rm $(BIN)

maid:
	rm ./*~ */*~
