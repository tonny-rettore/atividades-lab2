CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lm

INCLUDES = -I.

SRCS = main.c \
       Calculadora/calc.c \
       Dicionario/dic_simples.c \
       Lista/lista.c \
       String/str.c \
       Utf8/utf8.c

PROG = t3

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(PROG) $(LDLIBS)

clean:
	rm -f $(PROG)

.PHONY: clean