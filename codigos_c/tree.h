/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#ifndef _H_TREE
#define _H_TREE

#define QTD_TIPOS 24

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
#define VARIAVEL                22
#define IDENTIFICADOR           23

#define TIPO_LOG 0
#define TIPO_INT 1

typedef struct no *ptno;

struct no {
    int tipo;
    int valor;
    ptno filho, irmao;
};

ptno criaNo(int tipo, int valor);
void adicionaFilho(ptno pai, ptno filho);
void geraDot(FILE* dot, ptno arvore);
void desalocarArvore(ptno raiz);
void geraCodigo(FILE* arq, ptno p);

#endif