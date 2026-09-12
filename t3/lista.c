#include "lista.h"

#include <stdio.h>
#include <stdlib.h>

struct no
{
    dado_t dado;
    struct no *ant;
    struct no *prox;
};

struct lista
{
    struct no *sentinela;
    int tam;
};

Lista l_cria()
{
    Lista l = malloc(sizeof(struct lista));
    l->sentinela = malloc(sizeof(struct no));

    l->sentinela->ant = l->sentinela;
    l->sentinela->prox = l->sentinela;
    l->sentinela->dado = NULL;

    l->tam = 0;

    return l;
}

// cria uma lista contendo substrings de s
// as substrings são separadas por quaisquer caractere de sep
// os caracteres de sep não aparecem nas substrings
// exemplos:
//   "a,ba,ca, te", ", " -> ["a" "ba" "ca" "te"]
//   "aba \ncate\n", "\n" -> ["aba " "cate"]
// Lista l_cria_separando(Str s, Str sep){
  
// }

// libera a memória ocupada por uma lista
void l_destroi(Lista l)
{
    No atual = l->sentinela->prox;
    No prox;
    while (atual != l->sentinela)
    {
        prox = atual->prox;
        free(atual);
        atual = prox;
    }

    free(l->sentinela);
    free(l);
}

// retorna o número de elementos na lista
int l_tam(Lista l)
{
    return l->tam;
}

// retorna true se a lista tiver cheia
bool l_cheia(Lista l)
{
    return false;
}

// retorna true se a lista tiver vazia
bool l_vazia(Lista l)
{
    return l->tam == 0;
}

// imprime os dados que estão na lista
void l_imprime(Lista l)
{
    No atual = l->sentinela->prox;
    while (atual != l->sentinela)
    {
        s_imprime(atual->dado);
        atual = atual->prox;
    }
}

// insere o dado d no início da lista l
void l_insere_inicio(Lista l, dado_t d)
{
    No novo = malloc(sizeof(struct no));

    novo->dado = d;

    novo->prox = l->sentinela->prox;
    novo->ant = l->sentinela;

    l->sentinela->prox->ant = novo;
    l->sentinela->prox = novo;

    l->tam++;
}

// insere o dado d no final da lista l
void l_insere_fim(Lista l, dado_t d)
{
    No novo = malloc(sizeof(struct no));

    novo->dado = d;

    novo->prox = l->sentinela;
    novo->ant = l->sentinela->ant;

    l->sentinela->ant->prox = novo;
    l->sentinela->ant = novo;

    l->tam++;
}

// insere o dado d na lista l, de forma que ele fique na posição p
// a primeira posição é 0
void l_insere_pos(Lista l, dado_t d, int p)
{
    No atual;
    No novo;

    if (p < 0 || p > l->tam)
        return;

    atual = l->sentinela;

    for (int i = 0; i < p; i++)
        atual = atual->prox;

    novo = malloc(sizeof(struct no));

    novo->dado = d;

    novo->ant = atual;
    novo->prox = atual->prox;

    atual->prox->ant = novo;
    atual->prox = novo;

    l->tam++;
}

// retorna o dado no início da lista
dado_t l_dado_inicio(Lista l)
{
    return l->sentinela->prox->dado;
}

// retorna o dado no final da lista
dado_t l_dado_fim(Lista l)
{
    return l->sentinela->ant->dado;
}

// retorna o dado na posição pos da lista
dado_t l_dado_pos(Lista l, int pos)
{
    No atual;

    if (pos < 0 || pos >= l->tam)
        return NULL;

    atual = l->sentinela->prox;

    for (int i = 0; i < pos; i++)
        atual = atual->prox;

    return atual->dado;
}

// remove e retorna o dado no início da lista
dado_t l_remove_inicio(Lista l)
{
    if (l_vazia(l))
        return NULL;

    dado_t temp = l_dado_inicio(l);

    No inicio = l->sentinela->prox;

    l->sentinela->prox = inicio->prox;

    inicio->prox->ant = l->sentinela;

    free(inicio);

    l->tam--;

    return temp;
}

// remove e retorna o dado no final da lista
dado_t l_remove_fim(Lista l)
{
    if (l_vazia(l))
        return NULL;

    dado_t temp = l_dado_fim(l);

    No fim = l->sentinela->ant;

    l->sentinela->ant = fim->ant;
    fim->ant->prox = l->sentinela; 

    free(fim);

    l->tam--;

    return temp;
}

// remove e retorna o dado na posição pos da lista
dado_t l_remove_pos(Lista l, int pos)
{
    if (l_vazia(l))
        return NULL;

    if (pos < 0 || pos >= l->tam)
        return NULL;

    dado_t temp = l_dado_pos(l, pos);

    No atual = l->sentinela->prox;

    for (int i = 0; i < pos; i++)
        atual = atual->prox;
    
    atual->prox->ant = atual->ant;
    atual->ant->prox = atual->prox;

    free(atual);

    l->tam--;

    return temp;
}

// funções para usar a lista como uma fila

// l_cria, l_destroi, l_vazia

// retorna o dado que está no início da fila
dado_t l_primeiro(Lista l){
       return l_dado_inicio(l);
}

// insere um dado no fim da fila
void l_insere(Lista l, dado_t d){
    l_insere_fim(l, d);
}

// remove e retorna o dado que está no início da fila
dado_t l_remove(Lista l){
    return l_remove_inicio(l);
}


// funções para usar a lista como uma pilha

// l_cria, l_destroi, l_vazia

// retorna o dado que está no topo da pilha
dado_t l_topo(Lista l){
    return l_dado_fim(l);
}

// empilha um dado no topo da pilha
void l_empilha(Lista l, dado_t d){
    l_insere_fim(l, d);
}

// remove e retorna o dado que está no topo da pilha
dado_t l_desempilha(Lista l){
    return l_remove_fim(l);
}