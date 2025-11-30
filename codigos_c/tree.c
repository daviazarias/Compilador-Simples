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
#include "utils.h"
#include "tree.h"

#define logico(x)   ((x == 1) ? 'V' : 'F')

extern elemTabSimb tabSimb[];

static void escreverNos(FILE*,ptno);
static void escreverLigacoes(FILE*,ptno);

extern int n_identificadores;
static int nRotulos = 0;
static int aux;

#define N_VARIAVEIS n_identificadores

static char tipos[QTD_TIPOS][32] = 
    {"programa",                // 0
     "declaracao de variaveis", // 1
     "lista comandos",          // 2
     "tipo",                    // 3  - VALOR ENUM (0 ou 1)
     "lista variaveis",         // 4
     "leitura",                 // 5
     "escrita",                 // 6
     "repeticao",               // 7
     "selecao",                 // 8
     "atribuicao",              // 9
     "multiplicacao",           // 10
     "divisao",                 // 11
     "soma",                    // 12
     "subtracao",               // 13
     "compara maior",           // 14
     "compara menor",           // 15
     "compara igual",           // 16
     "e",                       // 17
     "ou",                      // 18
     "nao",                     // 19
     "numero",                  // 20 - VALOR NUMERICO
     "logico",                  // 21 - VALOR LOGICO (0 ou 1)
     "variavel",                // 22 - VALOR STRING
     "identificador"            // 23 - VALOR STRING
    };      

ptno criaNo(int tipo, int valor){
    ptno n = (ptno) malloc(sizeof(struct no));
    n->tipo = tipo;
    n->valor = valor;
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

    fprintf(dot, "\tn%p [label = \"%s | ", raiz, tipos[raiz->tipo]);

    switch(raiz->tipo){

        case NUMERO:
            fprintf(dot, "%d\"];\n", raiz->valor);
        break;

        case LOGICO:
            fprintf(dot, "%c\"];\n", logico(raiz->valor));
        break;
        
        case TIPO:
            fprintf(dot, "%s\"];\n", (raiz->valor == TIPO_INT) ? "inteiro" : "logico");    
        break;

        case VARIAVEL:
        case IDENTIFICADOR:
            fprintf(dot, "%s\"];\n", tabSimb[raiz->valor + 1].id);
        break;

        default:
            fprintf(dot, "\"];\n");
        break;
    }

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

void desalocarArvore(ptno raiz){
    
    if(!raiz) return;

    for(ptno n = raiz->filho; n; n = n->irmao)
        desalocarArvore(n);

    free(raiz);
}

// --------------------- GERAÇÃO DE CÓDIGO -------------------------

void geraCodigo(FILE *arq, ptno p){

    if(!p) return;

    ptno p1 = NULL;
    ptno p2 = NULL;
    ptno p3 = NULL;

    switch(p->tipo){

        case PROGRAMA:
            p1 = p->filho;
            p2 = p1->irmao;

            // Caso o programa não tenha comandos (sintaticamente correto)
            if(p2) p3 = p2->irmao;
            
            fprintf(arq, "\tINPP\n");

            if(N_VARIAVEIS)
                fprintf(arq, "\tAMEM\t%d\n", N_VARIAVEIS);

            // Se o programa não tiver variáveis, a lista
            // de comandos estará em p2, enquanto p3 é nulo
            geraCodigo(arq, p3 ? p3 : p2);

            if(N_VARIAVEIS)
                fprintf(arq, "\tDMEM\t%d\n", N_VARIAVEIS);

            fprintf(arq, "\tFIMP");

        break;

        case LISTA_COMANDOS:
            p1 = p->filho;
            p2 = p1->irmao;
            
            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
        break;

        case LEITURA:
            p1 = p->filho;
            fprintf(arq, "\tLEIA\n\tARZG\t%d\n", p1->valor);
        break;

        case ESCRITA:
            p1 = p->filho;
            geraCodigo(arq, p1);
            fprintf(arq, "\tESCR\n");
        break;

        case REPETICAO:
            p1 = p->filho;
            p2 = p1->irmao;

            fprintf(arq, "L%d\tNADA\n", empilha(nRotulos++));
            geraCodigo(arq, p1);

            fprintf(arq, "\tDSVF\tL%d\n", empilha(nRotulos++));
            geraCodigo(arq, p2);

            aux = desempilha();
            fprintf(arq, "\tDSVS\tL%d\n", desempilha());
            fprintf(arq, "L%d\tNADA\n", aux);
        break;

        case SELECAO:
            p1 = p->filho;
            p2 = p1->irmao;

            // Caso o bloco "então" seja vazio (sintaticamente correto)
            if(p2) p3 = p2->irmao;

            geraCodigo(arq, p1);

            fprintf(arq, "\tDSVF\tL%d\n", empilha(nRotulos++));

            geraCodigo(arq, p2);

            aux = desempilha();
            fprintf(arq, "\tDSVS\tL%d\n", empilha(nRotulos++));
            fprintf(arq, "L%d\tNADA\n", aux);

            geraCodigo(arq, p3);

            fprintf(arq, "L%d\tNADA\n", desempilha());

        break;

        case ATRIBUICAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p2);
            fprintf(arq, "\tARZG\t%d\n", p1->valor);
        break;

        case MULTIPLICACAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tMULT\n");
        break;

        case DIVISAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tDIVI\n");
        break;

        case SOMA:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tSOMA\n");
        break;

        case SUBTRACAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tSUBT\n");
        break;

        case COMPARA_MAIOR:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMMA\n");
        break;

        case COMPARA_MENOR:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMME\n");
        break;

        case COMPARA_IGUAL:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMIG\n");
        break;

        case CONJUNCAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCONJ\n");
        break;

        case DISJUNCAO:
            p1 = p->filho;
            p2 = p1->irmao;

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tDISJ\n");
        break;

        case NEGACAO:
            p1 = p->filho;

            geraCodigo(arq, p1);
            fprintf(arq, "\tNEGA\n");
        break;

        case NUMERO:
        case LOGICO:
            fprintf(arq, "\tCRCT\t%d\n", p->valor);
        break;

        case VARIAVEL:
            fprintf(arq, "\tCRVG\t%d\n", p->valor);
        break;
    }
}