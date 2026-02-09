CFLAGS  := -ggdb -Wall -std=c23 -pedantic -lcurses

SRC_DIR := src
OBJ_DIR := src/obj
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

UI_SRCS := $(wildcard $(SRC_DIR)/ui/*.c $(SRC_DIR)/ui/**/*.c)
OBJS += $(UI_SRCS:$(SRC_DIR)/ui/%.c=$(OBJ_DIR)/ui/%.o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/ui/%.o: $(SRC_DIR)/ui/%.c $(SRC_DIR)/ui/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/widget/%.o: $(SRC_DIR)/widget/%.c $(SRC_DIR)/widget/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/ui/modals/%.o: $(SRC_DIR)/ui/modals/%.c $(SRC_DIR)/ui/modals/%.h
	$(CC) $(CFLAGS) -c $< -o $@

bbs-client: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCS)
	$(CC) -MM $^ > $(SRC_DIR)/$@

clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/**/*.o $(OBJ_DIR)/ui/**/*.o $(BIN_DIR)/bbs-client

run:
	$(BIN_DIR)/bbs-client

valgrind:
	valgrind --tool=memcheck $(BIN_DIR)/bbs-client