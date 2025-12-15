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

#define texto(x) (x ? x : "")

static void escreverNos(FILE*,ptno);
static void escreverLigacoes(FILE*,ptno);

extern void yyerror(char*);
extern int yylineno;
extern elemTabSimb tabSimb[];

static int nRotulos = 1;
static int aux;

static char tipos[QTD_TIPOS][32] = 
    {"programa",                // 0
     "declaracao de variaveis", // 1
     "lista comandos",          // 2
     "tipo",                    // 3
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
     "referencia",              // 30
     "parametro"                // 31
    };      

    // inserirSimbolo(
    //             criarSimbolo(atomo, // Identificador
    //             GLOBAL,             // Escopo
    //             n_identificadores,  // Deslocamento
    //             VAZIO,              // Rótulo
    //             VAR,                // Categoria
    //             tipo,               // Tipo
    //             VAZIO               // Mecanismo
    //             )
    //         );     
     
ptno criaNo(int tipo, int retorno, char* id){
    ptno n = (ptno) malloc(sizeof(struct no));
    n->tipo = tipo;
    n->retorno = retorno;
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

void desalocarArvore(ptno raiz){
    
    if(!raiz) return;

    for(ptno n = raiz->filho; n; n = n->irmao)
        desalocarArvore(n);

    if(raiz->id) free(raiz->id);
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
        
            p2 = p->filho->irmao;

            fprintf(arq, "\tINPP\n");

            if(p2->tipo == DECLARACAO_VARIAVEIS)
                fprintf(arq, "\tAMEM\t%d\n", 0);

            for(ptno p_i = p2; p_i; p_i = p_i->irmao)
            {
                if(p_i->tipo == LISTA_COMANDOS)
                    fprintf(arq, "L0\tNADA\n");

                geraCodigo(arq, p_i);
            }

            if(p2->tipo == DECLARACAO_VARIAVEIS)
                fprintf(arq, "\tDMEM\t%d\n", 0);

            fprintf(arq, "\tFIMP");

        break;

        case LISTA_ROTINAS:
            p1 = p->filho;
            p2 = p1->irmao;

            fprintf(arq, "\tDSVS\tL0\n");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
        break;

        case FUNCAO:
            fprintf(arq, "L%d\tENSP\n", nRotulos++);

            for(p1 = p->filho; p1->irmao; p1 = p1->irmao);
            geraCodigo(arq, p1);

            fprintf(arq, "\tRTSP\t%d\n", 1 /* FIXME */);
        break;

        case PROCEDIMENTO:
            fprintf(arq, "L%d\tENSP\n", nRotulos++);

            for(p1 = p->filho; p1->irmao; p1 = p1->irmao);
            geraCodigo(arq, p1);

            fprintf(arq, "\tRTSP\t%d\n", 0);
        break;

        case CHAMADA_FUNCAO:
            fprintf(arq, "\tAMEM\t1\n");
            geraCodigo(arq, p->filho->irmao);

            fprintf(arq, "\tSVCP\n");
            fprintf(arq, "\tDSVS\tL%d\n", 0);
        break;

        case CHAMADA_PROCEDIMENTO:
            geraCodigo(arq, p->filho->irmao);
            fprintf(arq, "\tSVCP\n");
            fprintf(arq, "\tDSVS\tL%d\n", 0);
        break;

        case LISTA_ARGUMENTOS:
            geraCodigo(arq, p->filho);
            geraCodigo(arq, p->filho->irmao);
        break;

        case LISTA_COMANDOS:
            p1 = p->filho;
            p2 = p1->irmao;
            
            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
        break;

        case LEITURA:
            p1 = p->filho;
            
            fprintf(arq, "\tLEIA\n");
        break;

        case ESCRITA:
            geraCodigo(arq, p->filho);
            fprintf(arq, "\tESCR\n");
        break;

        case REPETICAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != LOG)
                yyerror("Expressão de comando de repetição precisa ter valor lógico");

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

            // Caso o bloco "então" seja vazio
            if(p2) p3 = p2->irmao;

            if(p1->retorno != LOG)
                yyerror("Expressão de comando de seleção precisa ter valor lógico");

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

            if(p1->retorno != p2->retorno)
                yyerror("Comando de atribuição precisa ter tipos compatíveis");

            geraCodigo(arq, p2);

        break;

        case MULTIPLICACAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Somente valores inteiros podem ser multiplicados");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tMULT\n");
        break;

        case DIVISAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Somente valores inteiros podem ser divididos");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tDIVI\n");
        break;

        case SOMA:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Somente valores inteiros podem ser somados");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tSOMA\n");
        break;

        case SUBTRACAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Somente valores inteiros podem ser subtraídos");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tSUBT\n");
        break;

        case COMPARA_MAIOR:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Operador de maior precisa de dois valores inteiros");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMMA\n");
        break;

        case COMPARA_MENOR:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != INT || p2->retorno != INT)
                yyerror("Operador de menor precisa de dois valores inteiros");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMME\n");
        break;

        case COMPARA_IGUAL:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != p2->retorno)
                yyerror("Operador de igualdade precisa ter valores tipos compatíveis");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCMIG\n");
        break;

        case CONJUNCAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != LOG || p2->retorno != LOG)
                yyerror("Operador de conjunção precisa de dois valores lógicos");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tCONJ\n");
        break;

        case DISJUNCAO:
            p1 = p->filho;
            p2 = p1->irmao;

            if(p1->retorno != LOG || p2->retorno != LOG)
                yyerror("Operador de disjunção precisa de dois valores lógicos");

            geraCodigo(arq, p1);
            geraCodigo(arq, p2);
            fprintf(arq, "\tDISJ\n");
        break;

        case NEGACAO:
            p1 = p->filho;

            if(p1->retorno != LOG)
                yyerror("Somente valores lógicos podem ser negados");

            geraCodigo(arq, p1);
            fprintf(arq, "\tNEGA\n");
        break;

        case NUMERO:
        case LOGICO:
            fprintf(arq, "\tCRCT\t%s\n", p->id);
        break;

        case IDENTIFICADOR:

        break;
    }
}