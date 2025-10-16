CFLAGS  := -ggdb -Wall -pedantic

SRC_DIR := src
OBJ_DIR := src/obj
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/main.h
	$(CC) $(CFLAGS) -c $< -o $@

bbs-client: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCS)
	$(CC) -MM $^ > $(SRC_DIR)/$@

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/bbs-client

run:
	$(BIN_DIR)/bbs-client

valgrind:
	valgrind --tool=memcheck $(BIN_DIR)/bbs-client