/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#ifndef _H_ARVORE
#define _H_ARVORE

#define QTD_TIPOS 32

#ifndef VAZIO
#define VAZIO -1
#endif

#define PROGRAMA                0
#define DECLARACAO_VARIAVEIS    1
#define LISTA_COMANDOS          2
#define TIPO                    3
#define LISTA_VARIAVEIS         4
#define LEITURA                 5
#define ESCRITA                 6
#define REPETICAO               7
#define SELECAO                 8
#define ATRIBUICAO              9
#define MULTIPLICACAO           10
#define DIVISAO                 11
#define SOMA                    12
#define SUBTRACAO               13
#define COMPARA_MAIOR           14
#define COMPARA_MENOR           15
#define COMPARA_IGUAL           16
#define CONJUNCAO               17
#define DISJUNCAO               18
#define NEGACAO                 19
#define NUMERO                  20
#define LOGICO                  21
#define IDENTIFICADOR           22
#define FUNCAO                  23
#define PROCEDIMENTO            24
#define LISTA_ROTINAS           25
#define CHAMADA_FUNCAO          26
#define CHAMADA_PROCEDIMENTO    27
#define LISTA_PARAMETROS        28
#define LISTA_ARGUMENTOS        29 
#define MECANISMO               30
#define PARAMETRO               31

typedef struct no *ptno;

struct no {
    int tipo;
    int valor;
    int linha;
    char* id;
    ptno filho, irmao;
};

ptno criaNo(int tipo, int valor, char* id);
void adicionaFilho(ptno pai, ptno filho);
void geraDot(FILE* dot, ptno arvore);
void desalocarArvore(ptno raiz);

#endif