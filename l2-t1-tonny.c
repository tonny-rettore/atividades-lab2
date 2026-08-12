#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// implementação de um cronômetro
typedef struct timespec crono;

typedef struct
{
    bool joga;
    bool onda;
    bool fim;

    bool diurno;

    int pontos;
    int municao;
    int vidas;
    int arma;
    int limite; // 10 ou 5
    int inimigos[10];
    int ataques;
    int chance;

    crono relogio;
    double intervalo_mov;
    double proximo_mov;

} estado_t;

// inicializa um cronômetro com a hora atual
void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

// retorna o tempo passado desde que o cronômetro *c foi iniciado, em segundos
double crono_parcial(crono *c)
{
    crono agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);

    double segundos = agora.tv_sec - c->tv_sec;
    double nanosegundos = agora.tv_nsec - c->tv_nsec;
    return segundos + 1e-9 * nanosegundos;
}

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
    est->joga = true;
    est->onda = false;
    est->fim = false;

    est->pontos = 0;
    est->municao = 30;
    est->vidas = 4;
    est->arma = 0;
    est->ataques = 20;
    est->chance = 100;

    est->intervalo_mov = 2;

    for (int i = 0; i < 10; i++)
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
    if (rand() % 100 < est->chance)
    {
        est->diurno = true;
        est->limite = 9;
    }
    else
    {
        est->diurno = false;
        est->limite = 4;
    }

    if (est->chance != 20)
        est->chance -= 20;
}

void troca_arma(estado_t *est)
{
    if (est->diurno)
    {
        est->arma++;

        if (est->arma > 10)
            est->arma = 0;
    }
    else
    {
        est->arma += 2;

        if (est->arma > 10)
            est->arma = 0;
    }
}

void atirar(estado_t *est)
{
    for (int i = 0; i <= est->limite; i++)
    {
        if (est->inimigos[i] == est->arma)
        {
            est->inimigos[i] = -1;
            break;
        }
    }
}

// sonar(){}

void processar_teclado_durante_onda(estado_t *est)
{
    int tecla = lechar();

    if (tecla == 9)
        troca_arma(est);
    if (tecla == 13)
        atirar(est);
    // if (tecla == 32)
    //     sonar();
    if (tecla == 27)
    {
        est->onda = false;
        est->joga = false;
        est->fim = true;
    }
}

void inimigo_chegou(estado_t *est)
{
    if (est->inimigos[0] != -1)
    {
        est->vidas--;

        if (est->vidas == 0)
        {
            est->joga = false;
        }
    }
}

void movimenta(estado_t *est)
{
    inimigo_chegou(est);
    for (int i = 0; i < est->limite; i++)
    {
        est->inimigos[i] = est->inimigos[i + 1];
    }
    if (est->ataques > 0)
    {
        int num = rand() % 11; // Gera um número entre 0 e 10
        est->inimigos[est->limite] = num;
        est->ataques--;
    }
}

void processar_tempo(estado_t *est)
{
    double agora = crono_parcial(&est->relogio);

    if (agora >= est->proximo_mov)
    {
        movimenta(est);
        est->proximo_mov = agora + est->intervalo_mov;
    }
}

void desenha(estado_t *est)
{
    if (est->arma == 10)
        printf("%d %d N", est->pontos, est->municao);
    else
        printf("%d %d %d", est->pontos, est->municao, est->arma);

    for (int i = 1; i < est->vidas; i++)
    {
        printf(")");
    }
    for (int i = 0; i < 10; i++)
    {
        if (est->inimigos[i] == -1)
        {
            printf(" ");
        }
        else
        {
            if (est->inimigos[i] == 10)
                printf("N");
            else
                printf("%d", est->inimigos[i]);
        }
    }
    printf("\r");
}

void inicializa_onda(estado_t *est)
{
    est->onda = true;
    crono_inicia(&est->relogio);

    est->proximo_mov = est->intervalo_mov;
}

void onda_acabou(estado_t *est)
{
    if (est->vidas == 0)
    {
        est->onda = false;
        return;
    }

    bool inimigos_vivos = false;

    for (int i = 0; i <= est->limite; i++)
    {
        if (est->inimigos[i] >= 0)
        {
            inimigos_vivos = true;
            break;
        }
    }
    if (est->ataques == 0 && inimigos_vivos == false)
    {
        est->onda = false;
    }
}

void joga_onda(estado_t *est)
{
    atualiza_dia_noite(est);
    inicializa_onda(est);

    while (est->onda)
    {
        processar_teclado_durante_onda(est);
        processar_tempo(est);
        desenha(est);
        onda_acabou(est);
    }
}

void menu(estado_t *est)
{
    printf("Pontuação atual: %d\n\nPressione r para começar a próxima onda ou ESC para sair", est->pontos);

    while (!est->onda && !est->fim)
    {
        char tecla_menu = lechar();
        if (tecla_menu == 27)
        {
            est->joga = false;
            est->fim = true;
        }
        if (tecla_menu == 'r')
        {
            est->onda = true;
        }
    }
}

void joga_partida(estado_t *est)
{
    while (est->joga)
    {
        joga_onda(est);
        menu(est);
    }
}

int main()
{
    srand(time(NULL));

    estado_t estado;

    inicializa_tela();
    inicializa_estado(&estado);
    while (!estado.fim)
    {
        joga_partida(&estado);
    }
    desinicializa_tela();
    return 0;
}
