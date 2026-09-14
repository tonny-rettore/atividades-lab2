#include "calc.h"

#include <stdio.h>
#include <stdlib.h>

//AUXILIARES

const char tabela(int topo, int entrada)
{
    static const char tab[5][6] =
    {
        {'T', 'E', 'E', 'E', 'E', 'R'},
        {'O', 'O', 'E', 'E', 'E', 'O'},
        {'O', 'O', 'O', 'E', 'E', 'O'},
        {'O', 'O', 'O', 'E', 'E', 'O'},
        {'R', 'E', 'E', 'E', 'E', 'D'}
    };

    return tab[topo][entrada];
}