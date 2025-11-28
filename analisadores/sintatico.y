/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "tree.h"

#define NOME_PROG "simples"

int yylex(void);
void yyerror(char*);

extern FILE *yyin, *yyout;
extern char atomo[];
extern int yylineno;
extern elemTabSimb tabSimb[];

elemTabSimb elemTab;

ptno raiz = NULL;
int n_identificadores = 0;
static int aux;
static int tipo;

%}

%code requires {
    #include "tree.h"
}

%union {
    ptno arvore;
    char* str;
}

%token T_PROGRAMA
%token T_INICIO
%token T_FIM
%token T_IDENTIF
%token T_LEIA
%token T_ESCREVA
%token T_ENQTO
%token T_FACA
%token T_FIMENQTO
%token T_SE
%token T_ENTAO
%token T_SENAO
%token T_FIMSE
%token T_ATRIB
%token T_VEZES
%token T_DIV
%token T_MAIS
%token T_MENOS
%token T_MAIOR
%token T_MENOR
%token T_IGUAL
%token T_E 
%token T_OU
%token T_V
%token T_F
%token T_ABRE 
%token T_FECHA
%token T_LOGICO
%token T_INTEIRO
%token T_NAO
%token T_NUM

%start programa

%left T_E T_OU
%left T_IGUAL
%left T_MAIOR T_MENOR
%left T_MAIS T_MENOS
%left T_VEZES T_DIV

%type <arvore> programa cabecalho variaveis declaracao_variaveis lista_comandos 
%type <arvore> tipo lista_variaveis comando leitura escrita repeticao selecao
%type <arvore> atribuicao expressao termo 

%%

programa
    : cabecalho variaveis T_INICIO lista_comandos T_FIM
        {
            $$ = criaNo(PROGRAMA, -1);
            adicionaFilho($$, $4);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);

            raiz = $$;
        }
    ;

cabecalho
    : T_PROGRAMA T_IDENTIF 
        {
            strcpy(elemTab.id, atomo);
            elemTab.end = -1;
            elemTab.tip = -1;
            inserirSimbolo(elemTab);

            $$ = criaNo(IDENTIFICADOR, -1);
        }
    ;

variaveis
    : {$$ = NULL;}
    | declaracao_variaveis {$$ = $1;}
    ;

declaracao_variaveis
    : tipo lista_variaveis declaracao_variaveis
        {
            $$ = criaNo(DECLARACAO_VARIAVEIS, -1);

            adicionaFilho($$,$3);
            adicionaFilho($$,$2);
            adicionaFilho($$,$1);
        }
    | tipo lista_variaveis
        {
            $$ = criaNo(DECLARACAO_VARIAVEIS, -1);

            adicionaFilho($$,$2);
            adicionaFilho($$,$1);
        }
    ;

tipo
    : T_LOGICO  { $$ = criaNo(TIPO, TIPO_LOG); tipo = TIPO_LOG; }
    | T_INTEIRO { $$ = criaNo(TIPO, TIPO_INT); tipo = TIPO_INT; }
    ;

lista_variaveis
    : lista_variaveis T_IDENTIF 
        { 
          strcpy(elemTab.id, atomo); 
          elemTab.end = n_identificadores;
          elemTab.tip = tipo;
          inserirSimbolo(elemTab);

          $$ = criaNo(LISTA_VARIAVEIS, -1);

          adicionaFilho($$, criaNo(VARIAVEL, n_identificadores++));
          adicionaFilho($$, $1);
        }     

    | T_IDENTIF
        { 
          strcpy(elemTab.id, atomo); 
          elemTab.end = n_identificadores;
          elemTab.tip = tipo;
          inserirSimbolo(elemTab);

          $$ = criaNo(LISTA_VARIAVEIS, -1);

          adicionaFilho($$, criaNo(VARIAVEL, n_identificadores++));
        }     
    ;

lista_comandos
    : /* vazia */ { $$ = NULL; }
    | comando lista_comandos
        {
            $$ = criaNo(LISTA_COMANDOS, -1);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    ;

comando
    : leitura       {$$ = $1;}
    | escrita       {$$ = $1;}
    | atribuicao    {$$ = $1;}
    | repeticao     {$$ = $1;}
    | selecao       {$$ = $1;}
    ;

leitura
    : T_LEIA T_IDENTIF
        { 
            $$ = criaNo(LEITURA, -1); 
            adicionaFilho($$, criaNo(VARIAVEL, tabSimb[buscaSimbolo(atomo)].end));
        }
    ;

escrita
    : T_ESCREVA expressao
        {
            desempilha();

            $$ = criaNo(ESCRITA, -1);
            adicionaFilho($$, $2);
        }
    ;

repeticao
    : T_ENQTO expressao
        {
            if(desempilha() != TIPO_LOG)
                yyerror("Estrutura de repetição precisa de expressão lógica");
        }  
        T_FACA lista_comandos T_FIMENQTO
        {
            $$ = criaNo(REPETICAO, -1);
            adicionaFilho($$, $5);
            adicionaFilho($$, $2);
        }
    ;

selecao
    : T_SE expressao
        {
            if(desempilha() != TIPO_LOG)
                yyerror("Estrutura de condição precisa de expressão lógica");
        }
    T_ENTAO lista_comandos T_SENAO lista_comandos T_FIMSE
        {
            $$ = criaNo(SELECAO, -1);
            adicionaFilho($$, $7);
            adicionaFilho($$, $5);
            adicionaFilho($$, $2);
        }
    ;

atribuicao
    : T_IDENTIF { aux = tabSimb[buscaSimbolo(atomo)].end; } T_ATRIB expressao
        { 
            if(desempilha() != tabSimb[aux + 1].tip)
                yyerror("Tipos incompatíveis na atribuição");

            $$ = criaNo(ATRIBUICAO, -1);
            adicionaFilho($$, $4);
            adicionaFilho($$, criaNo(VARIAVEL, aux));
        }
    ;

expressao
    : expressao T_VEZES expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_INT);

            $$ = criaNo(MULTIPLICACAO, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_DIV expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_INT);
            
            $$ = criaNo(DIVISAO, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MAIS expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_INT);

            $$ = criaNo(SOMA, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MENOS expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_INT);

            $$ = criaNo(SUBTRACAO, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MAIOR expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_LOG);

            $$ = criaNo(COMPARA_MAIOR, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MENOR expressao
        { 
            testaTipos(TIPO_INT, TIPO_INT, TIPO_LOG);
            
            $$ = criaNo(COMPARA_MENOR, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_IGUAL expressao
        { 
            if(desempilha() != desempilha())
                yyerror("Comparando expressões de tipos distintos");

            empilha(TIPO_LOG);

            $$ = criaNo(COMPARA_IGUAL, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_E expressao
        { 
            testaTipos(TIPO_LOG, TIPO_LOG, TIPO_LOG);

            $$ = criaNo(CONJUNCAO, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_OU expressao
        { 
            testaTipos(TIPO_LOG, TIPO_LOG, TIPO_LOG);

            $$ = criaNo(DISJUNCAO, -1);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | termo { $$ = $1; }
    ;

termo
    : T_IDENTIF    
        { 
            elemTab = tabSimb[buscaSimbolo(atomo)];
            $$ = criaNo(VARIAVEL, elemTab.end); 
            empilha(elemTab.tip); 
        }
    | T_NUM         
        { 
            $$ = criaNo(NUMERO, atoi(atomo)); 
            empilha(TIPO_INT);      
        }
    | T_V           
        { 
            $$ = criaNo(LOGICO, 1);
            empilha(TIPO_LOG);    
        }
    | T_F           
        {
            $$ = criaNo(LOGICO, 0);  
            empilha(TIPO_LOG); 
        }
    | T_NAO termo   
        {
            if(espiarPilha() != TIPO_LOG)
                yyerror("Negação só pode ser aplicada a valores lógicos");

            $$ = criaNo(NEGACAO, -1);
            adicionaFilho($$, $2);
        }
    | T_ABRE expressao T_FECHA { $$ = $2; }
    ;

%%

void yyerror(char *msg) {
    fprintf(stderr, "%d: Erro: %s\n", yylineno, msg);
    desalocarArvore(raiz);
    exit(1);
}

int main(int argc, char **argv){

    char nameIn[64], nameMvs[64], nameDot[64], nameSvg[64], cmd[200];
    char *p;
    FILE *dot;

    if(argc < 2){
        fprintf(stderr, "Uso: ./" NOME_PROG " <nome_arquivo>\n");
        return 1;
    }

    if(p = strstr(argv[1], ".simples"))
        *p = 0; 

    strcpy(nameIn, "codigos_simples/");
    strcat(nameIn, argv[1]);

    strcpy(nameSvg, strcpy(nameDot, strcpy(nameMvs, argv[1])));
    
    if(!(yyin = fopen(strcat(nameIn, ".simples"), "r"))){

        char *arq = (char*) strstr(nameIn, argv[1]);

        if(!(yyin = fopen(arq, "r"))){
            fprintf(stderr, "ERRO: %s não existe\n", arq);
            return 2;
        }
    }

    yyout = fopen(strcat(nameMvs,".mvs"), "w");
    dot   = fopen(strcat(nameDot,".dot"), "w");

    yyparse();

    geraDot(dot, raiz);
    geraCodigo(yyout, raiz);

    sprintf(cmd, "dot -Tsvg %s -o %s &", nameDot, strcat(nameSvg, ".svg"));
    system(cmd);

    desalocarArvore(raiz);
    fclose(yyin);
    fclose(yyout);
    fclose(dot);

    return 0;
}

    
