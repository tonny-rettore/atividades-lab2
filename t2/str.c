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

    if (strC == NULL || strC[0] == '\0')
    {
        s->bytes = NULL;
        s->nbytes = 0;
        s->nalloc = 0;
        s->nunichars = 0;
        return s;
    }

    s->nbytes = strlen(strC);
    s->nunichars = u8_conta_unichar_nos_bytes(s->nbytes, (byte *)strC);

    if (s->nunichars == -1)
    {
        // UTF-8 inválido
        free(s);
        return s_cria(NULL);
    }

    s->nalloc = MIN_ALLOC;
    while (s->nalloc < s->nbytes)
        s->nalloc *= 2;

    s->bytes = malloc(s->nalloc);
    assert(s->bytes != NULL);

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

    fseek(arq, 0, SEEK_END);  // leva cursor pro final
    int tamanho = ftell(arq); // diz posicao do cursor ou seja tamanho do texto
    rewind(arq);              // retorna cursor pro inicio

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
        pos += s->nunichars + 1;
    if (pos < 0 || pos >= s->nunichars)
        return UNI_INV;

    byte *p = u8_avanca_unichar(s->bytes, pos);

    unichar c;

    u8_unichar_nos_bytes(s->nbytes, p, &c);

    return c;
}

// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
    s_ok(s);
    s_ok(sb);

    // se forem exatamente o mesmo ponteiro
    if (s == sb)
    {
        return true;
    }

    // Se o número de bytes ou caracteres for diferente
    if (s->nbytes != sb->nbytes || s->nunichars != sb->nunichars)
    {
        return false;
    }

    // Se ambas forem strings vazias (nbytes == 0)
    if (s->nbytes == 0)
    {
        return true;
    }

    // Compara os bytes em memória das duas strings
    return memcmp(s->bytes, sb->bytes, s->nbytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);

    if (pos < 0)
        pos += s->nunichars + 1;

    for (int i = pos; i < s->nunichars; i++)
    {
        for (int x = 0; x < sb->nunichars; x++)
        {
            if (s_ch(s, i) == s_ch(sb, x))
            {
                return i;
            }
        }
    }

    return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);

    if (pos < 0)
        pos += s->nunichars + 1;

    for (int i = pos; i < s->nunichars; i++)
    {
        bool pertence = false;

        for (int x = 0; x < sb->nunichars; x++)
        {
            if (s_ch(s, i) == s_ch(sb, x))
            {
                pertence = true;
                break;
            }
        }

        if (!pertence)
        {
            return i;
        }
    }

    return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);

    if (pos < 0)
        pos += s->nunichars + 1;

    for (int i = pos - 1; i >= 0; i--)
    {
        for (int x = 0; x < sb->nunichars; x++)
        {
            if (s_ch(s, i) == s_ch(sb, x))
            {
                return i;
            }
        }
    }

    return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
    s_ok(s);
    s_ok(sb);

    if (pos < 0)
        pos += s->nunichars + 1;

    for (int i = pos - 1; i >= 0; i--)
    {
        bool pertence = false;

        for (int x = 0; x < sb->nunichars; x++)
        {
            if (s_ch(s, i) == s_ch(sb, x))
            {
                pertence = true;
                break;
            }
        }

        if (!pertence)
        {
            return i;
        }
    }

    return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
    s_ok(s);
    s_ok(buscada);

    if (pos < 0)
        pos += s->nunichars + 1;

    int tam_buscada = s_tam(buscada);

    if (tam_buscada == 0)
        return pos;

    for (int i = pos; i + tam_buscada <= s_tam(s); i++)
    {
        bool igual = true;

        for (int x = 0; x < tam_buscada; x++)
        {
            if (s_ch(s, i + x) != s_ch(buscada, x))
            {
                igual = false;
                break;
            }
        }

        if (igual)
            return i;
    }

    return -1;
}

// operações de alteração {{{1
void s_substitui(Str s, int pos, int tam, Str_c sb)
{
    s_ok(s);

    // trata sb == NULL como string vazia
    int nb_sb = 0;
    byte *bytes_sb = NULL;

    if (sb != NULL)
    {
        s_ok(sb);
        nb_sb = sb->nbytes;
        bytes_sb = sb->bytes;
    }

    int n = s->nunichars;

    // resolve pos negativo
    if (pos < 0)
        pos += n + 1;

    // resolve tam negativo (até o final de s)
    int fim = (tam < 0) ? n : pos + tam;

    // corrige os limites do intervalo
    int inicio = pos;
    if (inicio < 0)
        inicio = 0;
    if (inicio > n)
        inicio = n;

    if (fim < inicio)
        fim = inicio;
    if (fim > n)
        fim = n;

    // converte posições de caractere para posições de byte
    int byte_inicio = (inicio == 0) ? 0 : (int)(u8_avanca_unichar(s->bytes, inicio) - s->bytes);
    int byte_fim = (fim == 0) ? 0 : (int)(u8_avanca_unichar(s->bytes, fim) - s->bytes);

    int novo_nbytes = s->nbytes - (byte_fim - byte_inicio) + nb_sb;

    byte *novo_bytes = NULL;
    int novo_nalloc = 0;

    if (novo_nbytes > 0)
    {
        novo_nalloc = MIN_ALLOC;
        while (novo_nalloc < novo_nbytes)
            novo_nalloc *= 2;

        novo_bytes = malloc(novo_nalloc);
        assert(novo_bytes != NULL);

        memcpy(novo_bytes, s->bytes, byte_inicio); // prefixo
        if (nb_sb > 0)
            memcpy(novo_bytes + byte_inicio, bytes_sb, nb_sb); // conteúdo novo
        memcpy(novo_bytes + byte_inicio + nb_sb,
               s->bytes + byte_fim, s->nbytes - byte_fim); // sufixo
    }

    free(s->bytes);

    s->bytes = novo_bytes;
    s->nbytes = (novo_nbytes > 0) ? novo_nbytes : 0;
    s->nalloc = novo_nalloc;
    s->nunichars = n - (fim - inicio) + (sb != NULL ? sb->nunichars : 0);
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
    s_ok(s);
    s_ok(sb);

    int n = sb->nunichars;

    // resolve pos negativo
    if (pos < 0)
        pos += n + 1;

    // resolve tam negativo (até o final de s)
    int fim = (tam < 0) ? n : pos + tam;

    // corrige os limites do intervalo
    int inicio = (pos < 0) ? 0 : (pos > n ? n : pos);
    fim = (fim < inicio) ? inicio : (fim > n ? n : fim);

    // converte posições de caractere para posições de byte
    int byte_inicio = (inicio > 0) ? (u8_avanca_unichar(sb->bytes, inicio) - sb->bytes) : 0;
    int byte_fim = (fim > 0) ? (u8_avanca_unichar(sb->bytes, fim) - sb->bytes) : 0;

    int novo_nbytes = byte_fim - byte_inicio;

    byte *novo_bytes = NULL;
    int novo_nalloc = 0;

    if (novo_nbytes > 0)
    {
        novo_nalloc = MIN_ALLOC;
        while (novo_nalloc < novo_nbytes)
            novo_nalloc *= 2;

        novo_bytes = malloc(novo_nalloc);
        assert(novo_bytes != NULL);

        memcpy(novo_bytes, sb->bytes + byte_inicio, novo_nbytes);
    }

    free(s->bytes);

    s->bytes = novo_bytes;
    s->nbytes = (novo_nbytes > 0) ? novo_nbytes : 0;
    s->nalloc = novo_nalloc;
    s->nunichars = fim - inicio;
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

    // converte c pra utf8
    byte buf[4];
    int nb_c = u8_converte_pra_utf8(c, buf);
    assert(nb_c >= 0);

    int n = s->nunichars;

    // resolve pos negativo
    if (pos < 0)
        pos += n + 1;

    // corrige limites
    if (pos < 0)
        pos = 0;
    if (pos > n)
        pos = n;

    // acha o offset em bytes de onde o char será inserido
    int byte_pos = 0;
    if (pos > 0)
        byte_pos = u8_avanca_unichar(s->bytes, pos) - s->bytes;

    int novo_nbytes = s->nbytes + nb_c;

    int novo_nalloc = MIN_ALLOC;
    while (novo_nalloc < novo_nbytes)
        novo_nalloc *= 2;

    byte *novo_bytes = malloc(novo_nalloc);
    assert(novo_bytes != NULL);

    memcpy(novo_bytes, s->bytes, byte_pos);                                          // prefixo
    memcpy(novo_bytes + byte_pos, buf, nb_c);                                        // char novo
    memcpy(novo_bytes + byte_pos + nb_c, s->bytes + byte_pos, s->nbytes - byte_pos); // sufixo

    free(s->bytes);

    s->bytes = novo_bytes;
    s->nbytes = novo_nbytes;
    s->nalloc = novo_nalloc;
    s->nunichars = n + 1;
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

    // primeira posição (a partir de 0) com um caractere que NÃO é sobra
    int inicio = s_busca_nc(s, 0, sobras);

    if (inicio == -1)
    {
        // toda a string é feita de caracteres de sobras (ou já era vazia)
        s_substitui(s, 0, -1, NULL);
        return;
    }

    // última posição (antes do fim) com um caractere que NÃO é sobra
    int fim = s_busca_rnc(s, s_tam(s), sobras);

    int tam = fim - inicio + 1;

    s_substring(s, s, inicio, tam);
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{

    s_ok(s);

    if (s->nbytes > 0)
        fwrite(s->bytes, 1, s->nbytes, stdout);
}

void s_grava_arquivo(Str_c s, char *nome)
{
    s_ok(s);

    FILE *arq = fopen(nome, "wb");

    if (arq == NULL)
        return;

    if (s->nbytes > 0)
        fwrite(s->bytes, 1, s->nbytes, arq);

    fclose(arq);
}

// vim: foldmethod=marker shiftwidth=2
