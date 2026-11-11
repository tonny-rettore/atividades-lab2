CC = gcc
LDLIBS = -lm

PROG = cobra
SRCS = main.c jogo.c fila.c terminal.c rede.c grafo.c auxiliar.c \
       individuo.c especie.c geracao.c treina.c
HEADERS = $(wildcard *.h)

.PHONY: all clean

all: $(PROG)

$(PROG): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDLIBS)

clean:
	rm -f $(PROG)
