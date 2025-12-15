/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#ifndef _H_UTILS
#define _H_UTILS

// AUSÊNCIA DE VALOR
#ifndef VAZIO
#define VAZIO -1
#endif

// ESCOPO
#define GLOBAL 0
#define LOCAL 1

// CATEGORIA
#define VAR 0
#define PRO 1
#define FUN 2
#define PAR 3

// MECANISMO
#define REF 0
#define VAL 1

typedef struct listaPars {
    int tip;
    int mec;
    struct listaPars *prox;
} listaPars;

typedef struct {
    char id[100];
    int esc;
    int dsl;
    int rot;
    int cat;
    int tip;
    int mec;
    listaPars *par;
} elemTabSimb;

elemTabSimb* criarSimbolo(char id[], int esc, int dsl, int rot, int cat, int tip, int mec);
listaPars** recuperarLista(int ind);
void atualizarDeslocamento(int indice, int dsl);
void inserirSimbolo(elemTabSimb* elem);
int buscaSimbolo(char *id);
int removerSimbolos(int qtd);

long empilha(long valor);
long desempilha(void);

listaPars* criarPar(int tip, int mec);
void inserirPar(listaPars** lista, listaPars* novoPar);

#endif