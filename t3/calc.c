#include "calc.h"

#include <stdio.h>
#include <stdlib.h>

// AUXILIARES

const char tabela(int topo, int entrada)
{
    static const char tab[5][6] =
        {
            {'T', 'E', 'E', 'E', 'E', 'R'},
            {'O', 'O', 'E', 'E', 'E', 'O'},
            {'O', 'O', 'O', 'E', 'E', 'O'},
            {'O', 'O', 'O', 'E', 'E', 'O'},
            {'R', 'E', 'E', 'E', 'E', 'D'}};

    return tab[topo][entrada];
}

static bool eh_espaco(unichar c)
{
    return c == ' ' || c == '\t' ||
           c == '\n' || c == '\r';
}

static bool eh_digito(unichar c)
{
    return c >= '0' && c <= '9';
}

static bool eh_letra(unichar c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static bool eh_inicio_identificador(unichar c)
{
    return eh_letra(c) || c == '_' || c == '$';
}

static bool eh_caractere_identificador(unichar c)
{
    return eh_letra(c) || eh_digito(c) ||
           c == '_' || c == '$';
}
static bool eh_operador(Str token)
{
    return s_igual(token, s_cria("+")) ||
           s_igual(token, s_cria("-")) ||
           s_igual(token, s_cria("*")) ||
           s_igual(token, s_cria("/")) ||
           s_igual(token, s_cria("^"));
}

