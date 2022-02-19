CC = cc
EXE = qik

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS = -Wall -Wextra -lm -lSDL2 -lconfig -O2
CPPFLAGS = -I$(INC_DIR)
DBGFLAGS = -O0 -g

all: $(EXE)

debug: CFLAGS += $(DBGFLAGS)
debug: all

$(EXE): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $(EXE) $(OBJ)

$(OBJ): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(EXE) $(OBJ_DIR)/*.o

maid:
	rm ./*~ */*~