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

typedef struct {
    char id[100];
    int end;
    int tip;
} elemTabSimb;

void inserirSimbolo(elemTabSimb elem);
int buscaSimbolo(char *id);
int empilha(int valor);
int desempilha(void);
int espiarPilha(void);
void testaTipos(int tipoA, int tipoB, int ret);

#endif