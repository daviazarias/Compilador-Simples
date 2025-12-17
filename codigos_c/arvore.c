/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

#include <stdlib.h>
#include <stdio.h>
#include "arvore.h"

#define texto(x) (x ? x : "")

static void escreverNos(FILE*,ptno);
static void escreverLigacoes(FILE*,ptno);

extern int yylineno;
extern void (*geracaoCodigo[])(FILE*,ptno);

static char tipos[QTD_TIPOS][32] = 
    {"programa",                // 00
     "declaracao_de_variaveis", // 01
     "lista_comandos",          // 02
     "tipo",                    // 03
     "lista_variaveis",         // 04
     "leitura",                 // 05
     "escrita",                 // 06
     "repeticao",               // 07
     "selecao",                 // 08
     "atribuicao",              // 09
     "multiplicacao",           // 10
     "divisao",                 // 11
     "soma",                    // 12
     "subtracao",               // 13
     "compara_maior",           // 14
     "compara_menor",           // 15
     "compara_igual",           // 16
     "e",                       // 17
     "ou",                      // 18
     "nao",                     // 19
     "numero",                  // 20
     "logico",                  // 21
     "identificador",           // 22
     "funcao",                  // 23
     "procedimento",            // 24
     "lista_rotinas",           // 25
     "chamada_funcao",          // 26
     "chamada_procedimento",    // 27
     "lista_parametros",        // 28
     "lista_argumentos",        // 29
     "mecanismo",               // 30
     "parametro"                // 31
    };    
    
// -------------------- MANIPULAÇÃO BÁSICA DA ÁRVORE -----------------------------
     
ptno criaNo(int tipo, int valor, char* id){
    ptno n = (ptno) malloc(sizeof(struct no));
    n->tipo = tipo;
    n->valor = valor;
    n->linha = yylineno;
    n->id = id;
    n->filho = n->irmao = NULL;
    return n;
}

void adicionaFilho(ptno pai, ptno filho){
    if(filho) {
        filho->irmao = pai->filho;
        pai->filho = filho;
    }
}

// -------------------- GERAÇÃO DO ARQUIVO .dot -------------------------

void geraDot(FILE* dot, ptno arvore){

    fprintf(dot, "digraph {\n\tnode [shape=record, height = .1];\n");
    escreverNos(dot, arvore);
    escreverLigacoes(dot, arvore);
    fprintf(dot, "}\n");
}

static void escreverNos(FILE* dot, ptno raiz){

    if(!raiz) return;

    fprintf(dot, "\tn%p [label = \"%s | %s\"]\n", raiz, tipos[raiz->tipo], texto(raiz->id));

    escreverNos(dot, raiz->filho);
    escreverNos(dot, raiz->irmao);
}

static void escreverLigacoes(FILE* dot, ptno raiz){

    if(!raiz || !(raiz->filho)) return;

    for(ptno n = raiz->filho; n; n = n->irmao){
        fprintf(dot, "\tn%p -> n%p;\n", raiz, n);
        escreverLigacoes(dot, n);
    }
}

// ------------------- GERENCIAMENTO DE RECURSOS -----------------------

void desalocarArvore(ptno raiz){
    
    if(!raiz) return;

    for(ptno n = raiz->filho; n; n = n->irmao)
        desalocarArvore(n);

    if(raiz->id) free(raiz->id);
    free(raiz);
}