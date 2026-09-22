#include "calc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    CAT_FIM,         // coluna F  -> 0
    CAT_MAIS_MENOS,  // coluna +- -> 1
    CAT_VEZES_DIV,   // coluna */ -> 2
    CAT_POT,         // coluna ^  -> 3
    CAT_ABRE_PAREN,  // coluna (  -> 4
    CAT_FECHA_PAREN, // coluna )  -> 5
    CAT_OPERANDO,    // não é coluna/linha da tabela
    CAT_ERRO         // não é coluna/linha da tabela
} Categoria;

// AUXILIARES
const char tabela(int topo, int entrada)
{
    static const char tab[5][6] =
        {
            //  F	   +-  * /	 ^	  (	   )    // p\e
            {'T', 'E', 'E', 'E', 'E', 'R'},  // V
            {'O', 'O', 'E', 'E', 'E', 'O'},  // +-
            {'O', 'O', 'O', 'E', 'E', 'O'},  // */
            {'O', 'O', 'O', 'E', 'E', 'O'},  // ^
            {'R', 'E', 'E', 'E', 'E', 'D'}}; // (

    return tab[topo][entrada];
}

static bool eh_espaco(unichar c)
{
    return c == ' ' || c == '\t' ||
           c == '\n' || c == '\r';
}

static bool eh_digito(unichar c)
{
    return c >= '0' && c <= '9';
}

static bool eh_letra(unichar c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

static bool eh_inicio_identificador(unichar c)
{
    return eh_letra(c) || c == '_' || c == '$';
}

static bool eh_caractere_identificador(unichar c)
{
    return eh_letra(c) || eh_digito(c) || c == '_' || c == '$';
}

static Categoria classifica(Str token)
{
    int tam = s_tam(token);

    unichar primeiro = s_ch(token, 0);

    if (tam == 1)
    {
        if (primeiro == '+' || primeiro == '-')
            return CAT_MAIS_MENOS;

        if (primeiro == '*' || primeiro == '/')
            return CAT_VEZES_DIV;

        if (primeiro == '^')
            return CAT_POT;

        if (primeiro == '(')
            return CAT_ABRE_PAREN;

        if (primeiro == ')')
            return CAT_FECHA_PAREN;
    }

    // operando começa com dígito, ., letra ou $
    if (eh_digito(primeiro) || primeiro == '.' || eh_inicio_identificador(primeiro))
        return CAT_OPERANDO;

    // não é operador conhecido nem operando válido
    return CAT_ERRO;
}

static Categoria classifica_topo(Lista pilha_op)
{
    if (l_vazia(pilha_op))
        return CAT_FIM;

    Str topo = l_topo(pilha_op);
    return classifica(topo);
}

static bool opera(Lista pilha_op, Lista pilha_oper)
{
    Str operador = l_desempilha(pilha_oper);

    if (l_tam(pilha_op) < 2)
    {
        s_destroi(operador);
        return false;
    }

    Str str_dir = l_desempilha(pilha_op);
    Str str_esq = l_desempilha(pilha_op);

    double esq = s_número(str_esq);
    double dir = s_número(str_dir);
    double res = 0;

    unichar op = s_ch(operador, 0);

    // pra div zero
    if (op == '/' && dir == 0)
    {
        s_destroi(str_esq);
        s_destroi(str_dir);
        s_destroi(operador);
        return false;
    }

    switch (op)
    {
        case '+': res = esq + dir; break;
        case '-': res = esq - dir; break;
        case '*': res = esq * dir; break;
        case '/': res = esq / dir; break;
        case '^': res = pow(esq, dir); break;
    }

    l_empilha(pilha_op, s_cria_número(res));

    s_destroi(str_esq);
    s_destroi(str_dir);
    s_destroi(operador);

    return true;
}

// PRINCIPAIS
Lista tokeniza(Str txt)
{
    Lista tokens = l_cria();

    int i = 0;
    int tam = s_tam(txt);

    while (i < tam)
    {
        unichar c = s_ch(txt, i);

        // ignora espaços
        if (eh_espaco(c))
        {
            i++;
            continue;
        }

        int inicio = i;
        i++;

        if (eh_digito(c) || c == '.')
        {
            while (i < tam && (eh_digito(s_ch(txt, i)) || s_ch(txt, i) == '.'))
                i++;
        }
        else if (eh_inicio_identificador(c))
        {
            while (i < tam && eh_caractere_identificador(s_ch(txt, i)))
                i++;
        }

        Str token = s_cria_substring(txt, inicio, i - inicio);
        l_insere_fim(tokens, token);
    }

    return tokens;
}

Str calculadora(Str expressão)
{
    Lista tokens = tokeniza(expressão);
    Lista pilha_op = l_cria();
    Lista pilha_oper = l_cria();

    int i = 0;
    int n = l_tam(tokens);
    Str resultado = NULL;

    while (resultado == NULL)
    {
        Categoria cat_entrada = (i < n) ? classifica(l_dado_pos(tokens, i)) : CAT_FIM;

        if (cat_entrada == CAT_OPERANDO)
        {
            l_empilha(pilha_op, s_cria_cópia(l_dado_pos(tokens, i)));
            i++;
            continue;
        }

        if (cat_entrada == CAT_ERRO)
        {
            resultado = s_cria("#ERRO token invalido");
            break;
        }

        Categoria cat_topo = classifica_topo(pilha_oper);
        char acao = tabela(cat_topo, cat_entrada);

        switch (acao)
        {
            case 'E':
                l_empilha(pilha_oper, s_cria_cópia(l_dado_pos(tokens, i)));
                i++;
                break;

            case 'O':
                if (!opera(pilha_op, pilha_oper))
                {
                    resultado = s_cria("#ERRO faltam operandos");
                }
                break;

            case 'D':
                {
                    Str paren = l_desempilha(pilha_oper);
                    s_destroi(paren);
                    i++;
                }
                break;

            case 'T':
                if (l_tam(pilha_op) == 1)
                    resultado = l_desempilha(pilha_op);
                else
                    resultado = s_cria("#ERRO expressao incompleta");
                break;

            case 'R':
                if (cat_entrada == CAT_FECHA_PAREN)
                    resultado = s_cria("#ERRO falta (");
                else
                    resultado = s_cria("#ERRO falta )");
                break;
        }
    }

    l_destroi(tokens);
    l_destroi(pilha_op);
    l_destroi(pilha_oper);

    return resultado;
}
