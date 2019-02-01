TARGET := obsw.out
SRCDIR := src/
BINDIR := bin/
SRC := $(wildcard $(SRCDIR)*.c)
OBJ := $(SRC:.c=.o)
CC := clang
CFLAGS := -O2 -std=c11 -Wall
LDFLAGS := -lm -lrt -lpthread

all wall: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $(BINDIR)$(TARGET)

wall: CFLAGS += -Wall -Werror

DEPS: $(SRC:%.c=%.deps)
-include $(DEPS)

%.deps: %.c
	$(CC) -c $(CFLAGS) $< -o $(SRCDIR)$@

clean:
	rm -f $(BINDIR)$(TARGET) $(DEPS) $(OBJ)

rebuild:
	make clean
	make all