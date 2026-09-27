#include "str.h"
#include "lista.h"
#include "calc.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("%s arquivo_entrada arquivo_saida\n", argv[0]);
        return 1;
    }

    Str conteudo = s_cria_de_arquivo(argv[1]);

    // cada linha = uma expressão
    Str sep = s_cria("\n");
    Lista linhas = l_cria_separando(conteudo, sep);
    s_destroi(sep);
    s_destroi(conteudo);

    // processa cada linha guardando os resultados em outra lista
    Lista resultados = l_cria();

    while (!l_vazia(linhas))
    {
        Str linha = l_remove_inicio(linhas);
        Str resultado = calculadora(linha);

        l_insere_fim(resultados, resultado);

        s_destroi(linha);
    }

    l_destroi(linhas);

    // junta os resultados numa única str, um por linha
    Str sep_saida = s_cria("\n");
    Str texto_saida = s_cria_unindo(resultados, sep_saida);
    s_destroi(sep_saida);

    s_grava_arquivo(texto_saida, argv[2]);

    s_destroi(texto_saida);
    l_destroi(resultados);

    return 0;
}