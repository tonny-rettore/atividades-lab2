#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    bool inicio;
    bool menu;
    bool onda;
    bool fim;

    int municao;
    int posicao[13];
    char arma[11];
    char inimigos[11];

} estado_t;

void inicializa_estado(estado_t *est)
{
    est->inicio = true;
    est->menu = false;
    est->onda = false;
    est->fim = false;

    est->municao = 30;

    for (int i = 0; i < 13; i++) {
        est->posicao[i] = i;
    }

    for (int i = 0; i < 11; i++) {
        est->arma[i] = i;
        est->inimigos[i] = i;
    }
}

void joga_onda(estado_t *est)
{
    while (est->onda)
    {
        processar_teclado(est);
        processar_tempo(est);
        apresenta(est);
    }
}

void joga_partida(estado_t *est)
{
    while (1)
    {
        joga_onda(est);
    }
}

int main()
{
    estado_t estado;
    inicializa_tela();
    inicializa_estado(&estado);
    while (estado.fim){
        joga_partida(&estado);
    }
    desinicializa_tela();
    return 0;
}