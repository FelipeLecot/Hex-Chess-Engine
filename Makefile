CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter -O2 -g
SRCDIR = src
TARGET = hex_chess_engine

SOURCES = \
	$(SRCDIR)/coords.c \
	$(SRCDIR)/bitboards.c \
	$(SRCDIR)/magics.c \
	$(SRCDIR)/board.c \
	$(SRCDIR)/movegen.c \
	$(SRCDIR)/zobrist.c \
	$(SRCDIR)/evaluation.c \
	$(SRCDIR)/evaluatemove.c \
	$(SRCDIR)/search.c \
	$(SRCDIR)/tt.c \
	$(SRCDIR)/fen.c \
	$(SRCDIR)/san.c \
	$(SRCDIR)/render.c \
	$(SRCDIR)/utils.c \
	$(SRCDIR)/uci.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean
