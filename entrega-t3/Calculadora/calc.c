#include "Calculadora/calc.h"
#include "Dicionario/dicionario.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    CAT_FIM,         // coluna F  -> 0
    CAT_MAIS_MENOS,  // coluna +- -> 1
    CAT_VEZES_DIV,   // coluna */ -> 2
    CAT_POT,         // coluna ^  -> 3
    CAT_ABRE_PAREN,  // coluna (  -> 4
    CAT_FECHA_PAREN, // coluna )  -> 5
    CAT_IGUAL,       // coluna =  -> 6
    CAT_OPERANDO,    // não é coluna/linha da tabela
    CAT_ERRO         // não é coluna/linha da tabela
} Categoria;

typedef struct calc
{
    Dicionário dic;
} *Calc;

static Calc CALC = NULL;

// AUXILIARES
static char tabela(int topo, int entrada)
{
    static const char tab[7][7] =
        {
            //    F	   +-   */	 ^	  (     )   =      // p\e
            {'T', 'E', 'E', 'E', 'E', 'R', 'E'},  // V
            {'O', 'O', 'E', 'E', 'E', 'O', 'E'},  // +-
            {'O', 'O', 'O', 'E', 'E', 'O', 'E'},  // */
            {'O', 'O', 'O', 'E', 'E', 'O', 'E'},  // ^
            {'R', 'E', 'E', 'E', 'E', 'D', 'E'},  // (
            {'R', 'R', 'R', 'R', 'R', 'R', 'R'},  // )
            {'O', 'E', 'E', 'E', 'E', 'O', 'E'}}; // =

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

// funcoes pra ser usada no dicionario
static bool str_igual(chave_t a, chave_t b)
{
    Str sa = a;
    Str sb = b;
    return s_igual(sa, sb);
}

static bool str_menor(chave_t a, chave_t b)
{
    Str sa = a;
    Str sb = b;

    char *ca = s_strc(sa); // converte pra string c
    char *cb = s_strc(sb);

    bool resultado = strcmp(ca, cb) < 0;

    free(ca);
    free(cb);

    return resultado;
}

static void garante_quecalc_criado(void)
{
    if (CALC == NULL)
    {
        CALC = malloc(sizeof(*CALC));
        CALC->dic = dic_cria(str_menor, str_igual);
    }
}

static Categoria classifica(Str token)
{
    int tam = s_tam(token);

    unichar primeiro = s_ch(token, 0);

    if (tam == 1)
    {
        if (primeiro == '=')
            return CAT_IGUAL;
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

static double valor_de_operando(Str operando, bool *ok)
{
    unichar primeiro = s_ch(operando, 0);

    if (!eh_inicio_identificador(primeiro))
    {
        *ok = true;
        return s_número(operando);
    }

    // senão é variável
    garante_quecalc_criado();

    Str valor_str = dic_busca(CALC->dic, operando);

    if (valor_str == VALOR_NÃO_EXISTE)
    {
        *ok = false;
        return 0;
    }

    *ok = true;
    return s_número(valor_str);
}

static const char *opera(Lista pilha_op, Lista pilha_oper)
{
    Str operador = l_desempilha(pilha_oper);

    if (l_tam(pilha_op) < 2)
    {
        s_destroi(operador);
        return "#ERRO faltam operandos";
    }

    Str str_dir = l_desempilha(pilha_op);
    Str str_esq = l_desempilha(pilha_op);

    bool ok_esq, ok_dir;
    double esq = valor_de_operando(str_esq, &ok_esq);
    double dir = valor_de_operando(str_dir, &ok_dir);

    if (!ok_esq || !ok_dir)
    {
        s_destroi(str_esq);
        s_destroi(str_dir);
        s_destroi(operador);
        return "#ERRO variavel nao existe";
    }

    double res = 0;

    unichar op = s_ch(operador, 0);

    // pra div zero
    if (op == '/' && dir == 0)
    {
        s_destroi(str_esq);
        s_destroi(str_dir);
        s_destroi(operador);
        return "#ERRO divisao por zero";
    }

    switch (op)
    {
    case '+':
        res = esq + dir;
        break;
    case '-':
        res = esq - dir;
        break;
    case '*':
        res = esq * dir;
        break;
    case '/':
        res = esq / dir;
        break;
    case '^':
        res = pow(esq, dir);
        break;
    }

    l_empilha(pilha_op, s_cria_número(res));

    s_destroi(str_esq);
    s_destroi(str_dir);
    s_destroi(operador);

    return NULL;
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
static const char *atribui(Lista pilha_op, Lista pilha_oper)
{
    Str operador_igual = l_desempilha(pilha_oper); // pro =

    if (l_tam(pilha_op) < 2)
    {
        s_destroi(operador_igual);
        return "#ERRO faltam operandos";
    }

    Str str_valor = l_desempilha(pilha_op);
    Str str_nome = l_desempilha(pilha_op);

    bool ok;
    double valor = valor_de_operando(str_valor, &ok);

    if (!ok)
    {
        s_destroi(str_valor);
        s_destroi(str_nome);
        s_destroi(operador_igual);
        return "#ERRO variavel nao existe";
    }

    Str valor_como_str = s_cria_número(valor);

    // 2 retirado precisa ser nome válido
    unichar primeiro = s_ch(str_nome, 0);
    if (!eh_inicio_identificador(primeiro))
    {
        s_destroi(str_valor);
        s_destroi(str_nome);
        s_destroi(valor_como_str);
        s_destroi(operador_igual);
        return "#ERRO atribuicao invalida";
    }

    garante_quecalc_criado();

    Str valor_antigo = dic_insere(CALC->dic, str_nome, valor_como_str);

    if (valor_antigo != VALOR_NÃO_EXISTE)
    {
        s_destroi(valor_antigo);
        s_destroi(str_nome);
    }

    l_empilha(pilha_op, s_cria_cópia(valor_como_str));

    s_destroi(str_valor);
    s_destroi(operador_igual);

    return NULL;
}

Str calculadora(Str expressão)
{

    garante_quecalc_criado();

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
        {
            Str topo_oper = l_topo(pilha_oper);
            unichar c = s_ch(topo_oper, 0);

            const char *erro;

            if (c == '=')
            {
                erro = atribui(pilha_op, pilha_oper);
            }
            else
            {
                erro = opera(pilha_op, pilha_oper);
            }

            if (erro != NULL)
                resultado = s_cria(erro);
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
            {
                Str final = l_desempilha(pilha_op);
                bool ok;
                double valor = valor_de_operando(final, &ok);

                if (ok)
                {
                    resultado = s_cria_número(valor);
                    s_destroi(final);
                }
                else
                {
                    s_destroi(final);
                    resultado = s_cria("#ERRO variavel nao existe");
                }
            }
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
