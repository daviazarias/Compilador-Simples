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

#define TAM_TAB 100 

void yyerror(char*);

elemTabSimb tabSimb[TAM_TAB];
int topoTab = -1; // indica a última posição ocupada da tabSimb

// void exibirPars(listaPars *lista){
//     for(listaPars* l = lista; l; l = l->prox)
//         fprintf(stderr, "[mec: %d | tip: %d] -> ", l->mec, l->tip);
//     fprintf(stderr, "[/]");
// }

// void exibirTabela(void){
//     fprintf(stderr,"\n");
//     for(int i = 0; i <= topoTab; i++){
//         elemTabSimb e = tabSimb[i];
//         fprintf(stderr, "[id: %s | esc: %d | dsl: %d | rot: %d | cat: %d | tip: %d | mec: %d | pars: ",
//             e.id, e.esc, e.dsl, e.rot, e.cat, e.tip, e.mec);
//         exibirPars(e.par);
//         fprintf(stderr, "]\n");
//     }
//     fprintf(stderr,"\n");
// }

elemTabSimb* criarSimbolo(char id[], int esc, int dsl, int rot, int cat, int tip, int mec){
    
    elemTabSimb* entrada = (elemTabSimb*) malloc(sizeof(elemTabSimb));

    strcpy(entrada->id, id);
    entrada->esc = esc;
    entrada->dsl = dsl,
    entrada->rot = rot;
    entrada->cat = cat;
    entrada->tip = tip;
    entrada->mec = mec;
    entrada->par = NULL;

    return entrada;
}

void atualizarDeslocamento(int indice, int dsl){
    tabSimb[indice].dsl = dsl;
}

int buscaSimbolo(char *id){
    int i;

    for(i = topoTab; i >= 0 && strcmp(tabSimb[i].id, id); i--);

    if(i == -1) return -1;

    return i;
}

int inserirSimbolo(elemTabSimb* elem){

    int i;

    if(topoTab == TAM_TAB)
        yyerror("Quantidade máxima de variáveis excedida");

    for(i = topoTab; i >= 0 && (strcmp(tabSimb[i].id, elem->id) || tabSimb[i].esc != elem->esc); i--);

    if(i != -1) return -1;

    tabSimb[++topoTab] = *elem;

    return topoTab;
}

void removerSimbolos(int qtd){
    topoTab -= qtd;
}

listaPars** recuperarLista(int ind){
    return &tabSimb[ind].par;
}

// ---------------- PILHA DE RÓTULOS E TIPOS DE EXPRESSÕES --------------------

#define TAM_PIL 128

elemPilha Pilha[TAM_PIL];
int topoPil = 0;

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

// ---------------- LISTA DE PARÂMETROS --------------------

listaPars* criarPar(int tip, int mec){

    listaPars* par = (listaPars*) malloc(sizeof(listaPars));
    par->tip = tip;
    par->mec = mec;
    par->prox = NULL;

    return par;
}

void inserirPar(listaPars** lista, listaPars* novoPar){
    novoPar->prox = *lista;
    *lista = novoPar;
}