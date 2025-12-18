/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#ifndef _H_PILHA
#define _H_PILHA

typedef union {
    int rotulo;
    void* ponteiro;
} elemPilha;

int empilhaRot(int rotulo);
void* empilhaPtr(void* ptr);
elemPilha desempilha(void);

#endif