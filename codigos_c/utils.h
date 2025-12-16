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

// TIPO
#define LOG 0
#define INT 1

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

typedef union {
    int rotulo;
    listaPars **lista;
    listaPars *par;
} elemPilha;

elemTabSimb* criarSimbolo(char id[], int esc, int dsl, int rot, int cat, int tip, int mec);
listaPars** recuperarLista(int ind);
void atualizarDeslocamento(int indice, int dsl);
int inserirSimbolo(elemTabSimb* elem);
int buscaSimbolo(char *id);
void removerSimbolos(int qtd);

int empilhaRot(int rotulo);
listaPars** empilhaLista(listaPars** lista);
listaPars* empilhaPar(listaPars* par);
elemPilha desempilha(void);

listaPars* criarPar(int tip, int mec);
void inserirPar(listaPars** lista, listaPars* novoPar);

#endif