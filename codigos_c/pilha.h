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