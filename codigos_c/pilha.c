/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#include "pilha.h"

#define TAM_PIL 128

void yyerror(char*);

static elemPilha Pilha[TAM_PIL];
static int topoPil = 0;

int empilhaRot(int rotulo) {
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++].rotulo = rotulo;
    return rotulo;
}

void* empilhaPtr(void* ptr){
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++].ponteiro = ptr;
    return ptr;
}

elemPilha desempilha(void) {
    if(topoPil == 0)
        yyerror("Pilha vazia!");
    return Pilha[--topoPil];
}