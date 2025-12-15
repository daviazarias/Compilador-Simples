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
#include "tree.h"
#include "utils.h"

#define NOME_PROG "simples"

int yylex(void);
void yyerror(char*);

extern FILE *yyin, *yyout;
extern int yylineno;

static ptno raiz = NULL;

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
%token T_FUNC
%token T_FIMFUNC
%token T_PROC
%token T_FIMPROC
%token T_REF
%token T_CHAMADA

%start programa

%left T_E T_OU
%left T_IGUAL
%left T_MAIOR T_MENOR
%left T_MAIS T_MENOS
%left T_VEZES T_DIV

%type <arvore> programa cabecalho variaveis declaracao_variaveis lista_comandos 
%type <arvore> tipo lista_variaveis comando leitura escrita repeticao selecao
%type <arvore> atribuicao expressao termo 
%type <arvore> rotinas lista_rotinas rotina funcao procedimento chamada_funcao chamada_procedimento
%type <arvore> lista_argumentos argumento lista_parametros parametro mecanismo

%type <str> T_IDENTIF T_CHAMADA T_NUM

%%

programa
    : cabecalho variaveis rotinas T_INICIO lista_comandos T_FIM
        {
            $$ = criaNo(PROGRAMA, VAZIO, NULL);
            adicionaFilho($$, $5);
            adicionaFilho($$, $3);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);

            raiz = $$;
        }
    ;

cabecalho
    : T_PROGRAMA T_IDENTIF 
        {
            $$ = criaNo(IDENTIFICADOR, VAZIO, $2);
        }
    ;

variaveis
    :                       {$$ = NULL;}
    | declaracao_variaveis  {$$ = $1;}
    ;

declaracao_variaveis
    : tipo lista_variaveis declaracao_variaveis
        {
            $$ = criaNo(DECLARACAO_VARIAVEIS, VAZIO, NULL);

            adicionaFilho($$,$3);
            adicionaFilho($$,$2);
            adicionaFilho($$,$1);
        }
    | tipo lista_variaveis
        {
            $$ = criaNo(DECLARACAO_VARIAVEIS, VAZIO, NULL);

            adicionaFilho($$,$2);
            adicionaFilho($$,$1);
        }
    ;

tipo
    : T_LOGICO  { $$ = criaNo(TIPO, LOG, strdup("logico")); }
    | T_INTEIRO { $$ = criaNo(TIPO, INT, strdup("inteiro")); }
    ;

lista_variaveis
    : T_IDENTIF lista_variaveis 
        {        
            $$ = criaNo(LISTA_VARIAVEIS, VAZIO, NULL);

            adicionaFilho($$, $2);
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $1));
        }     

    | T_IDENTIF
        { 
            $$ = criaNo(LISTA_VARIAVEIS, VAZIO, NULL);

            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $1));
        }     
    ;

rotinas
    :               { $$ = NULL; }
    | lista_rotinas { $$ = $1;   }
    ;

lista_rotinas
    : rotina lista_rotinas
        {
            $$ = criaNo(LISTA_ROTINAS, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    | rotina 
        {
            $$ = criaNo(LISTA_ROTINAS, VAZIO, NULL);
            adicionaFilho($$, $1);
        }
    ;

rotina
    : funcao       { $$ = $1; }
    | procedimento { $$ = $1; }
    ;

funcao
    : T_FUNC tipo T_IDENTIF T_ABRE lista_parametros T_FECHA T_INICIO lista_comandos T_FIMFUNC
        {
            $$ = criaNo(FUNCAO, VAZIO, $3);
            adicionaFilho($$, $8);
            adicionaFilho($$, $5);
            adicionaFilho($$, $2);
        }
    ;

procedimento
    : T_PROC T_IDENTIF T_ABRE lista_parametros T_FECHA T_INICIO lista_comandos T_FIMPROC
        {
            $$ = criaNo(PROCEDIMENTO, VAZIO, $2);
            adicionaFilho($$, $7);
            adicionaFilho($$, $4);
        }
    ;

lista_parametros
    : { $$ = NULL; }
    | parametro lista_parametros
        {
            $$ = criaNo(LISTA_PARAMETROS, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    ;

parametro
    : mecanismo tipo T_IDENTIF
        {
            $$ = criaNo(PARAMETRO, VAZIO, NULL);
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $3));
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    ;

mecanismo
    :       { $$ = criaNo(MECANISMO, VAL, strdup("valor")); }
    | T_REF { $$ = criaNo(MECANISMO, REF, strdup("referencia")); }
    ;

lista_comandos
    : /* vazia */ { $$ = NULL; }
    | comando lista_comandos 
        {
            $$ = criaNo(LISTA_COMANDOS, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    ;

comando
    : leitura               {$$ = $1;}
    | escrita               {$$ = $1;}
    | atribuicao            {$$ = $1;}
    | repeticao             {$$ = $1;}
    | selecao               {$$ = $1;}
    | chamada_procedimento  {$$ = $1;}
    ;

lista_argumentos
    : { $$ = NULL; }
    | argumento lista_argumentos
        {
            $$ = criaNo(LISTA_ARGUMENTOS, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, $1);
        }
    ;

argumento
    : expressao { $$ = $1; }
    ;

leitura
    : T_LEIA T_IDENTIF
        {
            $$ = criaNo(LEITURA, VAZIO, NULL); 
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $2));
        }
    ;

escrita
    : T_ESCREVA expressao
        {
            $$ = criaNo(ESCRITA, VAZIO, NULL);
            adicionaFilho($$, $2);
        }
    ;

repeticao
    : T_ENQTO expressao T_FACA lista_comandos T_FIMENQTO
        {
            $$ = criaNo(REPETICAO, VAZIO, NULL);
            adicionaFilho($$, $4);
            adicionaFilho($$, $2);
        }
    ;

selecao
    : T_SE expressao T_ENTAO lista_comandos T_SENAO lista_comandos T_FIMSE
        {
            $$ = criaNo(SELECAO, VAZIO, NULL);
            adicionaFilho($$, $6);
            adicionaFilho($$, $4);
            adicionaFilho($$, $2);
        }
    ;

atribuicao
    : T_IDENTIF T_ATRIB expressao
        {
            $$ = criaNo(ATRIBUICAO, VAZIO, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $1));
        }
    ;

expressao
    : expressao T_VEZES expressao
        { 
            $$ = criaNo(MULTIPLICACAO, INT, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_DIV expressao
        { 
            $$ = criaNo(DIVISAO, INT, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MAIS expressao
        { 
            $$ = criaNo(SOMA, INT, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MENOS expressao
        { 
            $$ = criaNo(SUBTRACAO, INT, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MAIOR expressao
        { 
            $$ = criaNo(COMPARA_MAIOR, LOG, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_MENOR expressao
        { 
            $$ = criaNo(COMPARA_MENOR, LOG, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_IGUAL expressao
        { 
            $$ = criaNo(COMPARA_IGUAL, LOG, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_E expressao
        { 
            $$ = criaNo(CONJUNCAO, LOG, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | expressao T_OU expressao
        { 
            $$ = criaNo(DISJUNCAO, LOG, NULL);
            adicionaFilho($$, $3);
            adicionaFilho($$, $1);
        }
    | termo { $$ = $1; }
    ;

termo
    : T_IDENTIF                 { $$ = criaNo(IDENTIFICADOR, VAZIO, $1); } 
    | T_NUM                     { $$ = criaNo(NUMERO, INT, $1); }
    | T_V                       { $$ = criaNo(LOGICO, LOG, strdup("V")); }
    | T_F                       { $$ = criaNo(LOGICO, LOG, strdup("F")); }
    | T_NAO termo               { $$ = criaNo(NEGACAO, LOG, NULL); adicionaFilho($$, $2); }
    | T_ABRE expressao T_FECHA  { $$ = $2; }
    | chamada_funcao            { $$ = $1; }
    ;

chamada_procedimento
    : T_CHAMADA lista_argumentos T_FECHA
        {
            $$ = criaNo(CHAMADA_PROCEDIMENTO, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $1));
        }
    ;

chamada_funcao
    : T_CHAMADA lista_argumentos T_FECHA
        {
            $$ = criaNo(CHAMADA_FUNCAO, VAZIO, NULL);
            adicionaFilho($$, $2);
            adicionaFilho($$, criaNo(IDENTIFICADOR, VAZIO, $1));
        }
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

        char *arq = strstr(nameIn, argv[1]);

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