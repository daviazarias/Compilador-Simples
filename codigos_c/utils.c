/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

// Quantidade máxima de variáveis permitida pelo programa
#define MAX_VARIAVEIS 100

// A tabela reserva uma entrada (a primeira) para armazenar o identificador do 
// cabeçalho do programa-fonte
#define TAM_TAB 1 + MAX_VARIAVEIS 

void yyerror(char*);

elemTabSimb tabSimb[TAM_TAB];
int topoTab = -1; // indica a última posição ocupada da tabSimb

int buscaSimbolo(char *id){
    int i;

    for(i = topoTab; i >= 0 && strcmp(tabSimb[i].id, id); i--);

    if(i == -1){
        char msg[200];
        sprintf(msg, "O identificador %s não foi definido.", id);
        yyerror(msg);
    }

    return i;
}

void inserirSimbolo(elemTabSimb elem){

    int i;

    if(topoTab == TAM_TAB)
        yyerror("Quantidade máxima de variáveis excedida");

    for(i = topoTab; i >= 0 && strcmp(tabSimb[i].id, elem.id); i--);

    if(i != -1){
        char msg[200];
        sprintf(msg, "Dupla definição de %s", elem.id);
        yyerror(msg);
    }

    tabSimb[++topoTab] = elem;
}

// ---------------- PILHA DE RÓTULOS E TIPOS DE EXPRESSÕES --------------------

#define TAM_PIL 100

int Pilha[TAM_PIL];
int topoPil = 0;

int empilha(int valor) {
    if(topoPil == TAM_PIL)
        yyerror("Pilha cheia!");
    Pilha[topoPil++] = valor;
    return valor;
}

int desempilha(void) {
    if(topoPil == 0)
        yyerror("Pilha vazia!");
    return Pilha[--topoPil];
}

int espiarPilha(void){
    if(topoPil == 0)
        yyerror("Pilha vazia!");
    return Pilha[topoPil - 1];
}

void testaTipos(int tipoA, int tipoB, int ret){

    if(desempilha() != tipoB || desempilha() != tipoA)
        yyerror("Incompatibilidade de tipos");

    empilha(ret);
}