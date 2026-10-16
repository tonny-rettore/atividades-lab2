#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fila.h"

struct no
{
    void *dado;
    struct no *prox;
    struct no *ant;
};

struct fila
{
    struct no *inicio;
    struct no *fim;
    struct no *atual;
    int tam_do_dado;
    int direcao;
};

typedef struct no *No;


Fila f_cria(int tam_do_dado)
{
    if (tam_do_dado <= 0)
        return NULL;

    Fila self = malloc(sizeof(struct fila));

    if (self == NULL)
        return NULL;

    self->inicio = NULL;
    self->fim = NULL;
    self->atual = NULL;
    self->tam_do_dado = tam_do_dado;
    self->direcao = 1;

    return self;
}


void f_destrói(Fila self)
{
    if (self == NULL)
        return;

    No atual = self->inicio;

    while (atual != NULL)
    {
        No aux = atual->prox;

        free(atual->dado);
        free(atual);

        atual = aux;
    }

    free(self);
}


bool f_tá_vazia(Fila self)
{
    return self == NULL || self->inicio == NULL;
}


void f_remove(Fila self, void *pdado)
{
    if (f_tá_vazia(self))
        return;

    struct no *aux = self->inicio;

    if (pdado != NULL)
        memcpy(pdado, aux->dado, self->tam_do_dado);

    self->inicio = aux->prox;

    if (self->inicio == NULL)
    {
        self->fim = NULL;
    }
    else
    {
        self->inicio->ant = NULL;
    }

    free(aux->dado);
    free(aux);
}


void f_insere(Fila self, void *pdado)
{
    if (self == NULL || pdado == NULL)
        return;

    No novo = malloc(sizeof(struct no));

    if (novo == NULL)
        return;

    novo->dado = malloc(self->tam_do_dado);

    if (novo->dado == NULL)
    {
        free(novo);
        return;
    }

    memcpy(novo->dado, pdado, self->tam_do_dado);
    novo->prox = NULL;
    novo->ant = self->fim;

    if (self->inicio == NULL)
    {
        self->inicio = novo;
    }
    else
    {
        self->fim->prox = novo;
    }

    self->fim = novo;
}


void f_inicia_percurso(Fila self, int pos_inicial)
{
    if (self == NULL)
        return;

    self->atual = NULL;

    if (pos_inicial >= 0)
    {
        self->atual = self->inicio;
        self->direcao = 1;

        for (int i = 0; i < pos_inicial && self->atual != NULL; i++)
            self->atual = self->atual->prox;
    }
    else
    {
        self->atual = self->fim;
        self->direcao = -1;
        for (int i = -1; i > pos_inicial && self->atual != NULL; i--)
            self->atual = self->atual->ant;
    }
}


bool f_próximo(Fila self, void *pdado)
{
    if (self == NULL || self->atual == NULL)
        return false;

    if (pdado != NULL)
        memcpy(pdado, self->atual->dado, self->tam_do_dado);

    if (self->direcao == 1)
        self->atual = self->atual->prox;
    else
        self->atual = self->atual->ant;

    return true;
}