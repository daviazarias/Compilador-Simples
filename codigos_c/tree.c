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
#include <stdbool.h>
#include "utils.h"
#include "tree.h"

#define texto(x) (x ? x : "")

static void escreverNos(FILE*,ptno);
static void escreverLigacoes(FILE*,ptno);

extern void yyerror(char*);
extern int yylineno;
extern elemTabSimb tabSimb[];

static bool primeira_rotina = true;
static int n_variaveis = 0;
static int n_parametros = 0;
static int tipo;
static int n_rotulos = 1;
static int aux;

static listaPars **listaParametros;
static listaPars *parametro;

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
     "mecanismo",               // 30
     "parametro"                // 31
    };      
     
ptno criaNo(int tipo, int valor, char* id){
    ptno n = (ptno) malloc(sizeof(struct no));
    n->tipo = tipo;
    n->valor = valor;
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

    ptno p_i = p->filho;
    int simbolo;

    switch(p->tipo){

        case PROGRAMA:
        
            fprintf(arq, "\tINPP\n");

            for(p_i = p_i->irmao; p_i; p_i = p_i->irmao)
            {
                if (p_i->tipo == LISTA_COMANDOS)
                    fprintf(arq, "L0\tNADA\n");

                geraCodigo(arq, p_i);
            }

            if(n_variaveis > 0)
                fprintf(arq, "\tDMEM\t%d\n", n_variaveis);

            fprintf(arq, "\tFIMP");

        break;

        case DECLARACAO_VARIAVEIS:
            tipo = p_i->valor;
            geraCodigo(arq, p_i->irmao);
            geraCodigo(arq, p_i->irmao->irmao);
        break;

        case LISTA_VARIAVEIS:

            inserirSimbolo(
                criarSimbolo(
                    p_i->id,                // Identificador
                    GLOBAL,                 // Escopo
                    n_variaveis++,          // Deslocamento
                    VAZIO,                  // Rótulo       
                    VAR,                    // Categoria
                    tipo,                   // Tipo
                    VAZIO                   // Mecanismo
                )
            );   

            if(p_i->irmao) geraCodigo(arq, p_i->irmao);
            else fprintf(arq, "\tAMEM\t%d\n", n_variaveis);

        break;

        case LISTA_ROTINAS:

            if(primeira_rotina){
                fprintf(arq, "\tDSVS\tL0\n");
                primeira_rotina = false;
            }

            n_parametros = 0;

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            removerSimbolos(n_parametros);

        break;

        case LISTA_PARAMETROS:
            geraCodigo(arq, p_i->irmao);
            geraCodigo(arq, p_i);
        break;

        case PARAMETRO:
            inserirSimbolo(
                criarSimbolo(
                    p_i->irmao->irmao->id,  // Identificador
                    LOCAL,                  // Escopo
                    -(3 + n_parametros++),  // Deslocamento
                    VAZIO,                  // Rótulo       
                    PAR,                    // Categoria
                    p_i->irmao->valor,      // Tipo
                    p_i->valor              // Mecanismo
                )
            );

            inserirPar(listaParametros, criarPar(p_i->irmao->valor, p_i->valor));

        break;

        case FUNCAO:

            inserirSimbolo(
                criarSimbolo(
                    p->id,                  // Identificador
                    GLOBAL,                 // Escopo
                    VAZIO,                  // Deslocamento -- DESCONHECIDO
                    n_rotulos,              // Rótulo       
                    FUN,                    // Categoria
                    p_i->valor,             // Tipo
                    VAZIO                   // Mecanismo
                )
            );   
            
            fprintf(arq, "L%d\tENSP\n", n_rotulos++);

            simbolo = buscaSimbolo(p->id);
            listaParametros = recuperarLista(simbolo);

            geraCodigo(arq, p_i->irmao);

            atualizarDeslocamento(simbolo, -(3 + n_parametros));

            geraCodigo(arq, p_i->irmao->irmao);

            fprintf(arq, "\tRTSP\t%d\n", n_parametros);

        break;

        case PROCEDIMENTO:
            
            inserirSimbolo(
                criarSimbolo(
                    p->id,                  // Identificador
                    GLOBAL,                 // Escopo
                    VAZIO,                  // Deslocamento 
                    n_rotulos,              // Rótulo       
                    PRO,                    // Categoria
                    VAZIO,                  // Tipo
                    VAZIO                   // Mecanismo
                )
            );     
            
            fprintf(arq, "L%d\tENSP\n", n_rotulos++);

            simbolo = buscaSimbolo(p->id);
            listaParametros = recuperarLista(simbolo);

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            fprintf(arq, "\tRTSP\t%d\n", n_parametros);

        break;

        case CHAMADA_FUNCAO:
            fprintf(arq, "\tAMEM\t1\n");

            simbolo = buscaSimbolo(p_i->id);
            parametro = tabSimb[simbolo].par;

            p->valor = tabSimb[simbolo].tip;

            geraCodigo(arq, p_i->irmao);

            if(parametro)
                yyerror("Função chamada com poucos argumentos");

            fprintf(arq, "\tSVCP\n");
            fprintf(arq, "\tDSVS\tL%d\n", tabSimb[simbolo].rot);
        break;

        case CHAMADA_PROCEDIMENTO:

            simbolo = buscaSimbolo(p_i->id);
            parametro = tabSimb[simbolo].par;

            geraCodigo(arq, p_i->irmao);

            if(parametro)
                yyerror("Procedimento chamado com poucos argumentos");

            fprintf(arq, "\tSVCP\n");
            fprintf(arq, "\tDSVS\tL%d\n", tabSimb[simbolo].rot);
        break;

        case LISTA_ARGUMENTOS:

            if(!parametro)
                yyerror("Argumentos em excesso na chamada de rotina");

            switch(parametro->mec)
            {
                case VAL:

                    geraCodigo(arq, p_i);

                    if(parametro->tip != p_i->valor)
                        yyerror("Tipo do argumento incompatível com tipo do parâmetro");

                break;

                case REF:

                    simbolo = buscaSimbolo(p_i->id);

                    switch(tabSimb[simbolo].cat)
                    {
                        case VAR:
                            fprintf(arq, "\tCREG\t%d\n", tabSimb[simbolo].dsl);
                        break;

                        case FUN:
                        case PAR:
                            fprintf(arq, "\tCREL\t%d\n", tabSimb[simbolo].dsl);
                        break;

                        case PRO:
                            yyerror("Procedimentos não podem ser passadom como argumentos de rotinas");
                        break;
                    }

                    if(parametro->tip != tabSimb[simbolo].tip)
                        yyerror("Tipo do argumento incompatível com tipo do parâmetro");

                break;
            }
            
            parametro = parametro->prox;

            geraCodigo(arq, p_i->irmao);
        break;

        case LISTA_COMANDOS:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);
        break;

        case LEITURA:
            fprintf(arq, "\tLEIA\n");

            simbolo = buscaSimbolo(p_i->id);

            switch(tabSimb[simbolo].cat)
            {
                case VAR:
                    fprintf(arq, "\tARZG\t%d\n", tabSimb[simbolo].dsl);
                break;

                case FUN:
                    fprintf(arq, "\tARZL\t%d\n", tabSimb[simbolo].dsl);
                break;

                case PAR:
                    switch(tabSimb[simbolo].mec)
                    {
                        case VAL:
                            fprintf(arq, "\tARZL\t%d\n", tabSimb[simbolo].dsl);
                        break;

                        case REF:
                            fprintf(arq, "\tARMI\t%d\n", tabSimb[simbolo].dsl);
                        break;
                    }
                break;

                case PRO:
                    yyerror("Retornos de leituras não podem ser passadas para procedimentos");
                break;

            }
        break;

        case ESCRITA:
            geraCodigo(arq, p_i);
            
            fprintf(arq, "\tESCR\n");
        break;

        case REPETICAO:
            fprintf(arq, "L%d\tNADA\n", empilha(n_rotulos++));
            geraCodigo(arq, p_i);

            if(p_i->valor != LOG)
                yyerror("Expressão de comando de repetição precisa ter valor lógico");

            fprintf(arq, "\tDSVF\tL%d\n", empilha(n_rotulos++));
            geraCodigo(arq, p_i->irmao);

            aux = desempilha();
            fprintf(arq, "\tDSVS\tL%d\n", desempilha());
            fprintf(arq, "L%d\tNADA\n", aux);

        break;

        case SELECAO:

            // Gera código para avaliar a expressão lógica
            geraCodigo(arq, p_i);

            // Se o retorno da expressão não for lógica, dispara erro.
            if(p_i->valor != LOG)
                yyerror("Expressão de comando de seleção precisa ter valor lógico");

            fprintf(arq, "\tDSVF\tL%d\n", empilha(n_rotulos++));

            // Gera código do "então"
            geraCodigo(arq, p_i->irmao); 

            aux = desempilha();
            fprintf(arq, "\tDSVS\tL%d\n", empilha(n_rotulos++));
            fprintf(arq, "L%d\tNADA\n", aux);

            // Gera código do "senão", caso ele exista
            if(p_i->irmao) 
                geraCodigo(arq, p_i->irmao->irmao);

            fprintf(arq, "L%d\tNADA\n", desempilha());

        break;

        case ATRIBUICAO:
            geraCodigo(arq, p_i->irmao);

            simbolo = buscaSimbolo(p_i->id);

            if(tabSimb[simbolo].tip != p_i->irmao->valor)
                yyerror("Comando de atribuição precisa ter tipos compatíveis");

            switch(tabSimb[simbolo].cat)
            {
                case VAR:
                    fprintf(arq, "\tARZG\t%d\n", tabSimb[simbolo].dsl);
                break;

                case FUN:
                    fprintf(arq, "\tARZL\t%d\n", tabSimb[simbolo].dsl);
                break;

                case PAR:
                    switch(tabSimb[simbolo].mec)
                    {
                        case VAL:
                            fprintf(arq, "\tARZL\t%d\n", tabSimb[simbolo].dsl);
                        break;

                        case REF:
                            fprintf(arq, "\tARMI\t%d\n", tabSimb[simbolo].dsl);
                        break;
                    }
                break;

                case PRO:
                    yyerror("Procedimentos não podem ser utilizados do lado esquerdo de atribuições");
                break;
            }

        break;

        case MULTIPLICACAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Somente valores inteiros podem ser multiplicados");
            
            fprintf(arq, "\tMULT\n");
        break;

        case DIVISAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Somente valores inteiros podem ser divididos");
            
            fprintf(arq, "\tDIVI\n");
        break;

        case SOMA:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Somente valores inteiros podem ser somados");
            
            fprintf(arq, "\tSOMA\n");

        break;
        
        case SUBTRACAO:

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Somente valores inteiros podem ser subtraídos");
            
            fprintf(arq, "\tSUBT\n");
        break;

        case COMPARA_MAIOR:

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Operador de maior precisa de dois valores inteiros");

            fprintf(arq, "\tCMMA\n");

        break;

        case COMPARA_MENOR:

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                yyerror("Operador de menor precisa de dois valores inteiros");

            fprintf(arq, "\tCMME\n");

        break;

        case COMPARA_IGUAL:

            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != p_i->irmao->valor)
                yyerror("Operador de igualdade precisa ter valores tipos compatíveis");

            fprintf(arq, "\tCMIG\n");

        break;

        case CONJUNCAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != LOG || p_i->irmao->valor != LOG)
                yyerror("Operador de conjunção precisa de valores lógicos");
            
            fprintf(arq, "\tCONJ\n");

        break;

        case DISJUNCAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != LOG || p_i->irmao->valor != LOG)
                yyerror("Operador de disjunção precisa de valores lógicos");
            
            fprintf(arq, "\tDISJ\n");

        break;

        case NEGACAO:
            geraCodigo(arq, p_i);

            if(p_i->valor != LOG)
                yyerror("Somente valores lógicos podem ser negados");

            fprintf(arq, "\tNEGA\n");
        break;

        case NUMERO:
        case LOGICO:
            fprintf(arq, "\tCRCT\t%s\n", p->id);
        break;

        case IDENTIFICADOR:
            simbolo = buscaSimbolo(p->id);
            
            switch(tabSimb[simbolo].cat)
            {
                case VAR:
                    fprintf(arq, "\tCRVG\t%d\n", tabSimb[simbolo].dsl);
                break;

                case FUN:
                    fprintf(arq, "\tCRVL\t%d\n", tabSimb[simbolo].dsl);
                break;

                case PAR:
                    switch(tabSimb[simbolo].mec)
                    {
                        case VAL:
                            fprintf(arq, "\tCRVL\t%d\n", tabSimb[simbolo].dsl);
                        break;

                        case REF:
                            fprintf(arq, "\tCRVI\t%d\n", tabSimb[simbolo].dsl);
                        break;
                    }
                break;

                case PRO:
                    yyerror("Procedimentos não podem ser utilizados em expressões");
                break;
            }

        p->valor = tabSimb[simbolo].tip;

        break;
    }
}