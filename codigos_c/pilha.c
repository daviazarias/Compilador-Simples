#include "pilha.h"

#define TAM_PIL 128

void yyerror(char*);

static elemPilha Pilha[TAM_PIL];
static int topoPil = 0;

int empilhaRot(int rotulo) {
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++] = (elemPilha) rotulo;
    return rotulo;
}

listaPars** empilhaLista(listaPars** lista) {
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++] = (elemPilha) lista;
    return lista;
}

listaPars* empilhaPar(listaPars* par) {
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++] = (elemPilha) par;
    return par;
}

elemPilha desempilha(void) {
    if(topoPil == 0)
        yyerror("Pilha vazia!");
    return Pilha[--topoPil];
}