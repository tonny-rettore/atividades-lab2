// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8 // alocação mínima

struct str
{
    byte *bytes;   // conteúdo da string em UTF-8
    int nbytes;    // quantidade de bytes usados
    int nalloc;    // quantidade de bytes alocados
    int nunichars; // quantidade de caracteres Unicode
};

// A memória para conter os bytes de uma string deve ser alocada e/ou
//   realocada conforme a necessidade, cuidando para que a quantidade
//   de memória alocada seja sempre:
//   - nula (não alocada) se a string for vazia, ou
//   - não inferior ao necessário para armazenar os bytes da codificação utf8;
//   - não inferior à alocação mínima;
//   - não superior ao triplo do número de bytes necessários
//     (exceto quando for o mínimo);
//   - uma potência de 2.

// funções auxiliares {{{1

// verifica se a string cad está de acordo com a especificação
// aborta o programa se não tiver
static void s_ok(Str_c s)
{
    assert(s != NULL);
    assert(s->nbytes >= 0);
    assert(s->nunichars >= 0);
    assert(s->nalloc >= 0);

    if (s->nbytes == 0)
    {
        assert(s->bytes == NULL);
        assert(s->nalloc == 0);
        assert(s->nunichars == 0);
    }
}

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
    Str s = malloc(sizeof(*s));
    assert(s != NULL);

    // Número de bytes da string
    s->nbytes = strlen(strC);

    // String vazia
    if (s->nbytes == 0)
    {
        s->bytes = NULL;
        s->nalloc = 0;
        s->nunichars = 0;
        return s;
    }

    // Conta quantos caracteres Unicode existem
    int nbytes_temp = s->nbytes;
    s->nunichars = u8_conta_unichar_nos_bytes(&nbytes_temp, strC);

    // Alocação
    s->nalloc = MIN_ALLOC;
    while (s->nalloc < s->nbytes)
        s->nalloc *= 2;

    s->bytes = malloc(s->nalloc);
    assert(s->bytes != NULL);

    // Cópia da string para o novo lugar da memória alocado
    memcpy(s->bytes, strC, s->nbytes);

    return s;
}

void s_destroi(Str s)
{
    s_ok(s);

    free(s->bytes);
    free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
    Str nova = s_cria("");
    s_substring(nova, s, pos, tam);
    return nova;
}

Str s_cria_cópia(Str_c s)
{
    return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
    FILE *arq = fopen(nome, "r");

    if (arq == NULL)
        return s_cria("");

    fseek(arq, 0, SEEK_END); // leva cursor pro final
    int tamanho = ftell(arq); // diz posicao do cursor ou seja tamanho do texto
    rewind(arq); // retorna cursor pro inicio

    char *texto = malloc(tamanho + 1);

    if (texto == NULL)
    {
        fclose(arq);
        return s_cria("");
    }

    int n = fread(texto, 1, tamanho, arq);
    texto[n] = '\0';

    fclose(arq);

    Str s = s_cria(texto);

    free(texto);

    return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
    s_ok(s);
    return s->nunichars;
}

char *s_strc(Str_c s)
{
    s_ok(s);

    char *str = malloc(s->nbytes + 1);

    if (str == NULL)
        return NULL;

    memcpy(str, s->bytes, s->nbytes);

    str[s->nbytes] = '\0';

    return str;
}

unichar s_ch(Str_c s, int pos)
{
    s_ok(s);

    if (pos < 0)
        pos += s->nunichars;
    if (pos < 0 || pos >= s->nunichars)
        return UNI_INV;

    byte *p = u8_avanca_unichar(s->bytes, pos);

    unichar c;

    u8_unichar_nos_bytes(s->nbytes, p, &c);

    return c;

    // Nas funções abaixo, o argumento `pos` refere-se à posição de um
    //   caractere (e não de um byte) em uma string. Esse argumento deve ser
    //   interpretado da seguinte forma:
    //   - se ele for 0 representa a posição do primeiro caractere da string;
    //     se for 1 a do segundo etc
    //   - se ele for -1, representa a posição logo após o último caractere da string;
    //     se for -2, a posição do último caractere etc
}

// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return false;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
    return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
    s_ok(s);
    s_ok(buscada);
    //...
    return -1;
}

// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
    s_ok(s);
    s_ok(sb);
    //...
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
    s_ok(s);
    s_ok(sb);
    //...
}

void s_copia(Str s, Str_c sb)
{
    s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
    s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
    s_ok(s);
    //...
}

void s_anexa(Str s, Str_c sb)
{
    s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
    s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
    s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
    s_ok(s);
    s_ok(sobras);
    //...
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
    s_ok(s);
    //...
}

void s_grava_arquivo(Str_c s, char *nome)
{
    s_ok(s);
    //...
}

// vim: foldmethod=marker shiftwidth=2
