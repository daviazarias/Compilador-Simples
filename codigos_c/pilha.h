#ifndef _H_PILHA
#define _H_PILHA

#include "simbolos.h"

typedef union {
    int rotulo;
    listaPars **lista;
    listaPars *par;
} elemPilha;

int empilhaRot(int rotulo);
listaPars** empilhaLista(listaPars** lista);
listaPars* empilhaPar(listaPars* par);
elemPilha desempilha(void);

#endif