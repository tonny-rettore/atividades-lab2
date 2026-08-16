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
    int recordes[3];

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

// --FUNCOES CONTAGEM TEMPO--

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

// --FUNCOES LEITURA E ESCRITA ARQUIVO--

void le_arq(estado_t *est)
{
    FILE *f = fopen("recordes.txt", "r");
    if (f == NULL) {
        for (int i = 0; i < 3; i++)
            est->recordes[i] = 0;
        return;
    }

    for (int i = 0; i < 3; i++) {
        if (fscanf(f, "%d", &est->recordes[i]) != 1)
            est->recordes[i] = 0;
    }

    fclose(f);
}

void grava_arq(estado_t *est)
{
    FILE *f = fopen("recordes.txt", "w");
    if (f == NULL) {
        perror("erro ao gravar arquivo de recordes");
        return;
    }

    for (int i = 0; i < 3; i++)
        fprintf(f, "%d\n", est->recordes[i]);

    fclose(f);
}

// atualiza os recordes
void atualiza_recordes(int pontos, estado_t *est)
{
    for (int i = 0; i < 3; i++) {
        if (pontos > est->recordes[i]) {
            for (int j = 2; j > i; j--)
                est->recordes[j] = est->recordes[j - 1];
            est->recordes[i] = pontos;
            break;
        }
    }
}

// deixa terminal em estado cru
void inicializa_terminal()
{
    if (system("stty raw opost -echo min 0 time 1") != 0) {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    };
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

// coloca terminal pra estado normal

void desinicializa_terminal()
{
    system("stty sane");
}

// funcao que inicializa algumas das variaveis do estado do jogo

void inicializa_estado(estado_t *est)
{
    est->joga = true;
    est->onda = false;
    est->fim = false;

    est->pontos = 0;

    est->vidas = 4;

    est->chance = 100;

    for (int i = 0; i < 10; i++) {
        est->inimigos[i] = -1;
    }

    est->intervalo_mov_anterior = 2;
}

// --SONS--
// funcao usada por todas funcoes de sons para tocar
void toca_som(char *arquivo, bool esperar)
{
    char comando[100];

    if (esperar) {
        sprintf(comando, "paplay --latency-msec=1 Sons/%s", arquivo);
        system(comando);
    } else {
        sprintf(comando, "paplay --latency-msec=1 Sons/%s &", arquivo);
        system(comando);
    }
}

// som dos inimigos, armas e tiros
void toca_ataque(int tipo, bool esperar)
{
    if (tipo == ARMA_N || tipo == ARMA_n) {
        toca_som("11.3.wav", esperar);
    } else {
        char arquivo[20];

        sprintf(arquivo, "%d.3.wav", tipo);
        toca_som(arquivo, esperar);
    }
}

// som escudo
void toca_escudo(bool esperar)
{
    toca_som("12.3.wav", esperar);
}

// som espaço e tiro errado
void toca_espaco(bool esperar)
{
    toca_som("x.3.wav", esperar);
}

// som fim da onda
void toca_fim_onda()
{
    for (int i = 10; i >= 0; i -= 2)
        toca_ataque(i, true);
}

// funcao para ler tecla
char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1)
        return c;
    return 0;
}

// funcao que calcula propbabilidade de ser diurno ou noturno
void atualiza_dia_noite(estado_t *est)
{
    if (rand() % 100 < est->chance) {
        est->diurno = true;
        est->limite = 9;
    } else {
        est->diurno = false;
        est->limite = 4;
    }

    if (est->chance != 20)
        est->chance -= 20;
}

// funcao trocar a arma
void troca_arma(estado_t *est)
{
    if (est->diurno) {
        est->arma++;

        if (est->arma > ARMA_N)
            est->arma = 0;
    } else {
        est->arma += 2;

        if (est->arma > ARMA_N)
            est->arma = 0;
    }

    toca_ataque(est->arma, false);
}

// calcula os pontos obtidos ao destruir um inimigo
int calcula_pontos_atirar(estado_t *est, int pos, int tipo)
{
    int pontos = (est->limite + 1) - pos;

    if (tipo == ARMA_n)
        pontos *= 2;

    if (!est->diurno)
        pontos *= 2;

    return pontos;
}

// destrói o inimigo transformando sua posição em -1
void destroi_inimigo(estado_t *est, int pos)
{
    int tipo = est->inimigos[pos];

    est->pontos += calcula_pontos_atirar(est, pos, tipo);
    est->inimigos[pos] = -1;
}

// procura o inimigo que pode ser atingido pela arma
int procura_inimigo(estado_t *est)
{
    for (int i = 0; i <= est->limite; i++) {
        if (est->inimigos[i] == est->arma)
            return i;

        if (est->inimigos[i] == ARMA_n && est->arma == ARMA_N)
            return i;
    }

    return -1;
}

// realiza um disparo
void atirar(estado_t *est)
{
    if (est->municao <= 0)
        return;

    est->municao--;

    int pos = procura_inimigo(est);

    if (pos == -1) {
        toca_espaco(false);
        return;
    }

    toca_ataque(est->arma, false);

    if (est->inimigos[pos] == ARMA_N) {
        est->inimigos[pos] = ARMA_n;
        return;
    }

    destroi_inimigo(est, pos);
}

// funcao do sonar
void sonar(estado_t *est)
{
    for (int i = 0; i < est->vidas - 1; i++)
        toca_escudo(true);

    for (int i = 0; i <= est->limite; i++) {
        if (est->inimigos[i] == -1)
            toca_espaco(true);
        else
            toca_ataque(est->inimigos[i], true);
    }
}

// funcao processar o teclado DURANTE A ONDA
void processar_teclado_durante_onda(estado_t *est)
{
    int tecla = lechar();

    if (tecla == 9)
        troca_arma(est);
    if (tecla == 13)
        atirar(est);
    if (tecla == 32)
        sonar(est);
    if (tecla == 27) {
        est->onda = false;
        est->joga = false;
    }
}

// funcao que confere se inimigo tirou vida/escudo
void inimigo_chegou(estado_t *est)
{
    if (est->inimigos[0] != -1) {
        est->vidas--;

        if (est->vidas > 0)
            toca_escudo(false);

        if (est->vidas == 0) {
            est->joga = false;
        }
    }
}

// fucao que movimenta os inimigos, mexe vetor
void movimenta(estado_t *est)
{
    inimigo_chegou(est);
    for (int i = 0; i < est->limite; i++) {
        est->inimigos[i] = est->inimigos[i + 1];
    }
    if (est->ataques > 0) {
        int num;
        if (est->diurno)
            num = rand() % 11; // Gera um número entre 0 e 10
        else
            num = rand() % 6 * 2; // Gera um número 0 2 4 6 8 10
        est->inimigos[est->limite] = num;
        toca_ataque(num, false);
        est->ataques--;
    } else {
        est->inimigos[est->limite] = -1;
    }
}

// funcao processa o tempo
void processar_tempo(estado_t *est)
{
    double agora = crono_parcial(&est->relogio);

    if (agora >= est->proximo_mov) {
        movimenta(est);
        est->proximo_mov = agora + est->intervalo_mov_atual;
    }
}

// funcao que desenha pontos, tiros, arma e escudo
void desenha_informacoes(estado_t *est)
{
    if (est->arma == ARMA_N)
        printf("%d %d N", est->pontos, est->municao);
    else
        printf("%d %d %d", est->pontos, est->municao, est->arma);

    for (int i = 1; i < est->vidas; i++)
        printf(")");
}

// funcao que desenha os inimigos vindo
void desenha_inimigos(estado_t *est)
{
    for (int i = 0; i <= est->limite; i++) {
        if (est->inimigos[i] == -1) {
            printf(" ");
        } else {
            if (est->inimigos[i] == ARMA_N)
                printf("N");
            else if (est->inimigos[i] == ARMA_n)
                printf("n");
            else
                printf("%d", est->inimigos[i]);
        }
    }
}

// funcao principal de desenho
void desenha(estado_t *est)
{
    if (est->diurno) {
        desenha_informacoes(est);
        desenha_inimigos(est);
    } else // caso é noturno apenas mostra pontos
        printf("%d", est->pontos);
    printf("                    \r"); // espaços apagam sobras da linha
}

// funcao chamada no inicio de cada onda
void inicializa_onda(estado_t *est)
{
    est->municao = 30;
    est->arma = 0;

    if (est->diurno) {
        est->ataques = 20;
        est->intervalo_mov_atual = est->intervalo_mov_anterior;
    } else {
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

//--ONDA ACABOU?--
// funcao que confere se os ataques inimigos acabaram
bool ataques_acabaram(estado_t *est)
{
    if (est->ataques == 0) {
        if (est->diurno) {
            est->pontos += est->municao * 2;
            est->pontos += (est->vidas - 1) * 10;
        } else {
            est->pontos += est->municao * 4;
            est->pontos += (est->vidas - 1) * 20;
        }

        toca_fim_onda();
        est->onda = false;
        return true;
    }
    return false;
}

// funcao ve se jogador perdeu
bool jogador_perdeu(estado_t *est)
{
    if (est->vidas == 0) {
        est->onda = false;
        est->joga = false;
        toca_fim_onda();
        return true;
    }
    return false;
}

// funcao principal se onda acabou
void onda_acabou(estado_t *est)
{
    if (jogador_perdeu(est))
        return;

    for (int i = 0; i <= est->limite; i++) {
        if (est->inimigos[i] >= 0) {
            return;
        }
    }
    ataques_acabaram(est);
}

// funcao que começa a onda
void joga_onda(estado_t *est)
{
    atualiza_dia_noite(est);
    inicializa_onda(est);

    while (est->onda) {
        processar_teclado_durante_onda(est);
        processar_tempo(est);
        desenha(est);
        onda_acabou(est);
    }
}

// menu entre duas ondas, ou seja só chega aqui caso jogar tiver vencido onda
void menu_prox_onda(estado_t *est)
{
    printf("\n");
    system("clear");
    printf("Pontuação atual: %d | Pressione r para próxima onda ou ESC para sair pro menu principal", est->pontos);

    while (!est->onda && !est->fim) {
        char tecla_menu_prox_onda = lechar();
        if (tecla_menu_prox_onda == 27) {
            est->joga = false;
            return;
        }
        if (tecla_menu_prox_onda == 'r') {
            system("clear");
            est->onda = true;
        }
    }
}

// funcao principal da partida/jogando
void joga_partida(estado_t *est)
{
    while (est->joga) {
        joga_onda(est);
        if (est->joga)
            menu_prox_onda(est);
    }
}

// verifica se a pontuação está entre os três maiores recordes
bool eh_recorde(int pontos, estado_t *est)
{
    for (int i = 0; i < 3; i++) {
        if (pontos > est->recordes[i])
            return true;
    }

    return false;
}

// printfs do menu principal
void escreve_painel_menu_principal(estado_t *est)
{
    bool recorde = eh_recorde(est->pontos, est);

    printf("\nSua pontuação foi: %d\n", est->pontos);

    if (recorde)
        printf("Você entrou no Top 3!\n");
    else
        printf("Você não entrou no Top 3.\n");

    printf("Top 3 recordes: %d, %d, %d\n", est->recordes[0], est->recordes[1], est->recordes[2]);
    printf("Pressione r para jogar novamente ou ESC para sair");
}

// menu principal, aparece quando o jogador perde
void menu_principal(estado_t *est)
{
    printf("\n");
    system("clear");
    atualiza_recordes(est->pontos, est);
    grava_arq(est);

    escreve_painel_menu_principal(est);

    while (true) {
        char tecla = lechar();
        if (tecla == 27) {
            est->fim = true;
            return;
        }
        if (tecla == 'r') {
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

    le_arq(&estado);

    inicializa_terminal();
    while (!estado.fim) {
        inicializa_estado(&estado);
        joga_partida(&estado);
        menu_principal(&estado);
    }

    system("pkill paplay");
    desinicializa_terminal();
    return 0;
}