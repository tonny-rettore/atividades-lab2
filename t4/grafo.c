#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "grafo.h"
#include "fila.h"

#define CAPACIDADE_INICIAL 10

// tipos de consulta de arestas (usados por g_próxima_aresta)
#define CONSULTA_NENHUMA 0
#define CONSULTA_PARTEM 1
#define CONSULTA_CHEGAM 2

struct aresta
{
    int destino; // nó para onde a aresta aponta
    void *valor;
    struct aresta *prox; // próxima aresta que parte do mesmo nó
};

struct no_grafo
{
    void *valor;
    struct aresta *arestas; // lista das arestas que partem deste nó
};

struct grafo
{
    struct no_grafo *nos; // vetor de nós
    int num_nos;
    int capacidade;

    int tam_no;
    int tam_aresta;

    // usados para consulta das arestas, coloquei aqui para sobreviver a chamada das funções
    int tipo_consulta; // CONSULTA_NENHUMA, PARTEM ou CHEGAM
    int no_consulta;
    int no_varredura;
    struct aresta *aresta_atual;
};

typedef struct no_grafo *NoGrafo;
typedef struct aresta *Aresta;

Grafo g_cria(int tam_nó, int tam_aresta)
{
    if (tam_nó <= 0 || tam_aresta <= 0)
        return NULL;

    Grafo self = malloc(sizeof(struct grafo));
    if (self == NULL)
        return NULL;

    self->nos = malloc(CAPACIDADE_INICIAL * sizeof(struct no_grafo));
    if (self->nos == NULL)
    {
        free(self);
        return NULL;
    }

    self->num_nos = 0;
    self->capacidade = CAPACIDADE_INICIAL;
    self->tam_no = tam_nó;
    self->tam_aresta = tam_aresta;
    self->tipo_consulta = CONSULTA_NENHUMA;
    self->no_consulta = 0;
    self->no_varredura = 0;
    self->aresta_atual = NULL;

    return self;
}

void g_destrói(Grafo self)
{
    if (self == NULL)
        return;

    for (int i = 0; i < self->num_nos; i++)
    {
        free(self->nos[i].valor);

        Aresta aresta_atual = self->nos[i].arestas;
        while (aresta_atual != NULL)
        {
            Aresta prox = aresta_atual->prox;
            free(aresta_atual->valor);
            free(aresta_atual);
            aresta_atual = prox;
        }
    }

    free(self->nos);
    free(self);
}

// --------- NÓS --------------- //

int g_insere_nó(Grafo self, void *p_dado)
{
    if (self == NULL || p_dado == NULL)
        return -1;

    if (self->num_nos == self->capacidade)
    {
        int nova_cap = self->capacidade * 2;
        NoGrafo novo = realloc(self->nos, nova_cap * sizeof(struct no_grafo));
        if (novo == NULL)
            return -1;
        self->nos = novo;
        self->capacidade = nova_cap;
    }

    void *valor = malloc(self->tam_no);
    if (valor == NULL)
        return -1;
    memcpy(valor, p_dado, self->tam_no);

    self->nos[self->num_nos].valor = valor;
    self->nos[self->num_nos].arestas = NULL;

    return self->num_nos++;
}

void g_remove_nó(Grafo self, int nó)
{
    if (self == NULL || nó < 0 || nó >= self->num_nos)
        return;

    self->tipo_consulta = CONSULTA_NENHUMA;
    self->aresta_atual = NULL;

    // libera o valor do nó e as arestas que partem dele
    free(self->nos[nó].valor);
    Aresta a = self->nos[nó].arestas;
    while (a != NULL)
    {
        Aresta prox = a->prox;
        free(a->valor);
        free(a);
        a = prox;
    }

    // desloca os nós seguintes uma posição para trás
    for (int i = nó; i < self->num_nos - 1; i++)
        self->nos[i] = self->nos[i + 1];
    self->num_nos--;

    // arruma as arestas dos nós que restaram
    for (int i = 0; i < self->num_nos; i++)
    {
        Aresta atual = self->nos[i].arestas;
        Aresta ant = NULL;
        while (atual != NULL)
        {
            Aresta prox = atual->prox;
            if (atual->destino == nó)
            {
                if (ant == NULL)
                    self->nos[i].arestas = prox;
                else
                    ant->prox = prox;
                free(atual->valor);
                free(atual);
            }
            else
            {
                if (atual->destino > nó)
                    atual->destino--;
                ant = atual;
            }
            atual = prox;
        }
    }
}

void g_altera_valor_do_nó(Grafo self, int nó, void *p_dado)
{
    if (self == NULL || nó >= self->num_nos || nó < 0 || p_dado == NULL)
        return;

    memcpy(self->nos[nó].valor, p_dado, self->tam_no);
}

void *g_valor_do_nó(Grafo self, int nó)
{
    if (self == NULL || nó < 0 || nó >= self->num_nos)
        return NULL;

    return self->nos[nó].valor;
}

int g_num_nós(Grafo self)
{
    if (self == NULL)
        return -1;
    return self->num_nos;
}

// ----------- ARESTAS ------------ //

void g_altera_valor_da_aresta(Grafo self, int nó_origem, int nó_destino, void *p_dado)
{
    if (self == NULL || nó_origem < 0 || nó_origem >= self->num_nos || nó_destino < 0 || nó_destino >= self->num_nos)
        return;

    Aresta atual = self->nos[nó_origem].arestas;
    Aresta ant = NULL;
    while (atual != NULL)
    {
        if (atual->destino == nó_destino)
        {
            if (p_dado != NULL)
            {
                memcpy(atual->valor, p_dado, self->tam_aresta);
            }
            else
            {
                if (ant == NULL)
                    self->nos[nó_origem].arestas = atual->prox;
                else
                    ant->prox = atual->prox;
                free(atual->valor);
                free(atual);
            }
            return;
        }
        ant = atual;
        atual = atual->prox;
    }
    // se chegou aqui é pq nao existia

    if (p_dado == NULL)
        return;

    Aresta nova = malloc(sizeof(struct aresta));
    if (nova == NULL)
        return;

    nova->valor = malloc(self->tam_aresta);
    if (nova->valor == NULL)
    {
        free(nova);
        return;
    }
    memcpy(nova->valor, p_dado, self->tam_aresta);
    nova->destino = nó_destino;
    nova->prox = self->nos[nó_origem].arestas;
    self->nos[nó_origem].arestas = nova;
}

void *g_valor_da_aresta(Grafo self, int nó_origem, int nó_destino)
{
    if (self == NULL || nó_origem < 0 || nó_origem >= self->num_nos || nó_destino < 0 || nó_destino >= self->num_nos)
        return NULL;
    Aresta atual = self->nos[nó_origem].arestas;
    while (atual != NULL)
    {
        if (atual->destino == nó_destino)
            return atual->valor;
        else
            atual = atual->prox;
    }
    return NULL;
}

void g_arestas_que_partem(Grafo self, int nó_origem)
{
    if (self == NULL)
        return;

    if (nó_origem < 0 || nó_origem >= self->num_nos)
    {
        self->tipo_consulta = CONSULTA_NENHUMA;
        self->aresta_atual = NULL;
        return;
    }

    self->tipo_consulta = CONSULTA_PARTEM;
    self->no_consulta = nó_origem;
    self->aresta_atual = self->nos[nó_origem].arestas;
}

void g_arestas_que_chegam(Grafo self, int nó_destino)
{
    if (self == NULL)
        return;

    if (nó_destino < 0 || nó_destino >= self->num_nos)
    {
        self->tipo_consulta = CONSULTA_NENHUMA;
        self->aresta_atual = NULL;
        return;
    }

    self->tipo_consulta = CONSULTA_CHEGAM;
    self->no_consulta = nó_destino;
    self->no_varredura = 0;
    self->aresta_atual = self->nos[0].arestas;
}

void *g_próxima_aresta(Grafo self, int *p_nó_vizinho)
{
    if (self == NULL)
        return NULL;

    // modo partem anda na lista do nó consultado
    if (self->tipo_consulta == CONSULTA_PARTEM)
    {
        Aresta a = self->aresta_atual;
        if (a == NULL)
            return NULL;

        self->aresta_atual = a->prox;
        if (p_nó_vizinho != NULL)
            *p_nó_vizinho = a->destino;
        return a->valor;
    }

    // modo chegam varre a lista de cada nó, atrás de destino == no consulta
    if (self->tipo_consulta == CONSULTA_CHEGAM)
    {
        while (self->no_varredura < self->num_nos)
        {
            // anda na lista do nó que está sendo varrido
            while (self->aresta_atual != NULL)
            {
                Aresta a = self->aresta_atual;
                self->aresta_atual = a->prox;

                if (a->destino == self->no_consulta)
                {
                    if (p_nó_vizinho != NULL)
                        *p_nó_vizinho = self->no_varredura;
                    return a->valor;
                }
            }

            // passa para o próximo nó
            self->no_varredura++;
            if (self->no_varredura < self->num_nos)
                self->aresta_atual = self->nos[self->no_varredura].arestas;
        }
        return NULL;
    }

    return NULL;
}

// ----- ALGORITMOS -------- //
