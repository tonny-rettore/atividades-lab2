#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct
{
    bool inicio;
    bool menu;
    bool onda;
    bool fim;

    bool diurno;

    int pontos;
    int municao;
    int vidas;
    char arma;
    int limite; // 10 ou 5
    char inimigos[10];
    int ataques;
    float chance;

} estado_t;

void inicializa_tela()
{
    if (system("stty raw opost -echo min 0 time 1") != 0)
    {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    };
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0)
    {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

void desinicializa_tela()
{
    system("stty sane");
}

void inicializa_estado(estado_t *est)
{
    est->inicio = true;
    est->menu = false;
    est->onda = false;
    est->fim = false;

    est->pontos = 0;
    est->municao = 30;
    est->vidas = 3;
    est->arma = 0;
    est->ataques = 20;
    est->chance = 0;

    for (int i = 0; i > 10; i++)
    {
        est->inimigos[i] = -1;
    }
}

char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1)
        return c;
    return 0;
}

void atualiza_dia_noite(estado_t *est)
{
    srand(time(NULL));

    if (rand() % 100 < est->chance)
    {
        est->diurno = false;
        est->limite = 10;
    }
    else
    {
        est->diurno = true;
        est->limite = 5;
    }
}

void trocar_arma(estado_t *est)
{
    if (est->diurno)
    {
        if (est->arma == 10)
            est->arma = 0;
        else
            est->arma++;
    }
    else
    {
        if (est->arma == 10)
            est->arma = 0;
        else
            est->arma += 2;
    }
}

void atirar(estado_t *est)
{
    for (int i = 0; i < est->limite; i++)
    {
        if (est->inimigos[i] == est->arma)
            est->arma = -1;
        break;
    }
}

// sonar(){}

void processar_teclado(estado_t *est)
{
    int c = lechar;

    if (c == 9)
    {
        troca_arma(est);
    }
    if (c == 13)
    {
        atirar(est);
    }
    // if (c == 32)
    // {
    //     sonar();
    // }
}

void movimenta(estado_t *est)
{
    if (est->ataques <= 0)
    {
        return;
    }

    srand(time(NULL));
    int num = rand() % 11; // Gera um número entre 0 e 10

    for (int i = 0; i < est->limite; i++)
    {
        est->inimigos[i] = est->inimigos[i + 1];
    }
    est->inimigos[est->limite] = num;
}

void desenha(estado_t *est)
{
    if (est->arma == 10)
        est->arma = 'N';

    printf("%d %d %c", est->pontos, est->municao, est->arma);

    est->arma = 10;

    for (int i = 0; i < est->vidas; i++)
    {
        printf(")");
    }
    for (int i = 0; i < 10; i++)
    {
        if (est->inimigos == -1)
        {
            printf(" ");
        }
        else
        {
            printf("%c", est->inimigos[i]);
        }
    }
    printf("\r");
}

void joga_onda(estado_t *est)
{
    while (est->onda)
    {
        processar_teclado(est);
        processar_tempo(est);
        movimenta(est);
        desenha(est);
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
    while (estado.fim)
    {
        joga_partida(&estado);
    }
    desinicializa_tela();
    return 0;
}
