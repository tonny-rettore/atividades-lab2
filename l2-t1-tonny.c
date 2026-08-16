#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ARMA_N 10
#define ARMA_n 11

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
    double intervalo_mov_anterior;
    double intervalo_mov_atual;
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

void le_arq(){

}

void grava_arq(){
    
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

    est->vidas = 4;

    est->chance = 100;

    for (int i = 0; i < 10; i++)
    {
        est->inimigos[i] = -1;
    }

    est->intervalo_mov_anterior = 2;
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

        if (est->arma > ARMA_N)
            est->arma = 0;
    }
    else
    {
        est->arma += 2;

        if (est->arma > ARMA_N)
            est->arma = 0;
    }
}

void atirar(estado_t *est)
{
    if (est->municao <= 0)
        return;

    est->municao--;
    for (int i = 0; i <= est->limite; i++)
    {
        if (est->inimigos[i] == ARMA_n && est->arma == ARMA_N)
        {
            est->inimigos[i] = -1;
            if (est->diurno)
                est->pontos += ((est->limite + 1) - i) * 2;
            else
                est->pontos += ((est->limite + 1) - i) * 4;
            break;
        }
        if (est->inimigos[i] == est->arma)
        {
            if (est->inimigos[i] == ARMA_N)
            {
                est->inimigos[i] = ARMA_n;
            }
            else
            {
                est->inimigos[i] = -1;
                if (est->diurno)
                    est->pontos += (est->limite + 1) - i;
                else
                    est->pontos += ((est->limite + 1) - i) * 2;
            }
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
        int num;
        if (est->diurno)
            num = rand() % 11; // Gera um número entre 0 e 10
        else
            num = rand() % 6 * 2; // Gera um número 0 2 4 6 8 10
        est->inimigos[est->limite] = num;
        est->ataques--;
    }
    else
    {
        est->inimigos[est->limite] = -1;
    }
}

void processar_tempo(estado_t *est)
{
    double agora = crono_parcial(&est->relogio);

    if (agora >= est->proximo_mov)
    {
        movimenta(est);
        est->proximo_mov = agora + est->intervalo_mov_atual;
    }
}

void desenha_informacoes(estado_t *est)
{
    if (est->arma == ARMA_N)
        printf("%d %d N", est->pontos, est->municao);
    else
        printf("%d %d %d", est->pontos, est->municao, est->arma);

    for (int i = 1; i < est->vidas; i++)
    {
        printf(")");
    }
}

void desenha_inimigos(estado_t *est)
{
    for (int i = 0; i < 10; i++)
    {
        if (est->inimigos[i] == -1)
        {
            printf(" ");
        }
        else
        {
            if (est->inimigos[i] == ARMA_N)
                printf("N");
            else if (est->inimigos[i] == ARMA_n)
                printf("n");
            else
                printf("%d", est->inimigos[i]);
        }
    }
}

void desenha(estado_t *est)
{
    if (est->diurno)
    {
        desenha_informacoes(est);
        desenha_inimigos(est);
    }
    else
        printf("%d", est->pontos);
    printf("\r");
}

void inicializa_onda(estado_t *est)
{
    est->municao = 30;
    est->arma = 0;

    if (est->diurno)
    {
        est->ataques = 20;
        est->intervalo_mov_atual = est->intervalo_mov_anterior;
    }
    else
    {
        est->intervalo_mov_atual = est->intervalo_mov_anterior * 3;
        est->ataques = 15;
    }
    est->intervalo_mov_anterior *= 0.9;

    for (int i = 0; i < 10; i++)
        est->inimigos[i] = -1;

    est->onda = true;

    crono_inicia(&est->relogio);
    est->proximo_mov = est->intervalo_mov_atual;
}

void onda_acabou(estado_t *est)
{
    if (est->vidas == 0)
    {
        est->onda = false;
        est->joga = false;
        return;
    }

    bool inimigos_vivos = false;

    for (int i = 0; i <= est->limite; i++)
    {
        if (est->inimigos[i] >= 0)
        {
            inimigos_vivos = true;
            return;
        }
    }
    if (est->ataques == 0 && inimigos_vivos == false)
    {
        if (est->diurno)
        {
            est->pontos += est->municao * 2;
            est->pontos += (est->vidas - 1) * 10;
        }
        else
        {
            est->pontos += est->municao * 4;
            est->pontos += (est->vidas - 1) * 20;
        }

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

void menu_prox_onda(estado_t *est)
{
    printf("Pontuação atual: %d | Pressione r para próxima onda ou ESC para sair pro menu principal", est->pontos);

    while (!est->onda && !est->fim)
    {
        char tecla_menu_prox_onda = lechar();
        if (tecla_menu_prox_onda == 27)
        {
            est->joga = false;
        }
        if (tecla_menu_prox_onda == 'r')
        {
            system("clear");
            est->onda = true;
        }
    }
}

void joga_partida(estado_t *est)
{
    while (est->joga)
    {
        joga_onda(est);
        if (est->joga)
            menu_prox_onda(est);
    }
}

void menu_principal(estado_t *est)
{
    printf("Sua pontuação foi: %d\n", est->pontos);
    printf("Pressione r para jogar novamente ou ESC para sair");

    while (true)
    {
        char tecla = lechar();
        if (tecla == 27)
        {
            est->fim = true;
            return;
        }
        if (tecla == 'r')
        {
            system("clear");
            return;
        }
    }
}

int main()
{
    system("clear");
    srand(time(NULL));

    estado_t estado;
    estado.fim = false;

    inicializa_tela();
    while (!estado.fim)
    {
        inicializa_estado(&estado);
        joga_partida(&estado);
        menu_principal(&estado);
    }
    desinicializa_tela();
    return 0;
}
