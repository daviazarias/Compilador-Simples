/*+=============================================================
  |           UNIFAL -- Universidade Federal de Alfenas.
  |             BACHARELADO EM CIENCIA DA COMPUTACAO.
  | Trabalho..: Construcao Arvore Sintatica e Geracao de Codigo
  | Disciplina: Teoria de Linguagens e Compiladores
  | Professor.: Luiz Eduardo da Silva
  | Aluno.....: Davi Azarias do Vale Cabral
  | Data......: 23/11/2025
  +=============================================================*/

identificador  [a-zA-Z]([a-zA-Z0-9])*
numero         [0-9]+
espaco         [ \t\r]+
novalinha      [\n]

%x comment

%{
#include "sintatico.h"

void yyerror(char*);
char atomo[100];
%}

%%

programa        { return T_PROGRAMA; }
inicio          { return T_INICIO; }
fimprograma     { return T_FIM; }

leia            { return T_LEIA; }
escreva         { return T_ESCREVA; }

se              { return T_SE; }
entao           { return T_ENTAO; }
senao           { return T_SENAO; }
fimse           { return T_FIMSE; }

enquanto        { return T_ENQTO; }
faca            { return T_FACA; }
fimenquanto     { return T_FIMENQTO; }

"+"             { return T_MAIS; }
"-"             { return T_MENOS; }
"*"             { return T_VEZES; }
div             { return T_DIV; }

">"             { return T_MAIOR; }
"<"             { return T_MENOR; }
"="             { return T_IGUAL; }

e               { return T_E; }
ou              { return T_OU; }
nao             { return T_NAO; }

"<-"            { return T_ATRIB; }
"("             { return T_ABRE; }
")"             { return T_FECHA; }

inteiro         { return T_INTEIRO; }
logico          { return T_LOGICO; }
V               { return T_V; }
F               { return T_F; }

{identificador} {
                  strcpy(atomo, yytext);
                  return T_IDENTIF;
                }

{numero}        {
                  strcpy(atomo, yytext);
                  return T_NUM;
                }

{espaco}        /* nao faz nada */
{novalinha}     ++yylineno;
.               printf("%11s: ERRO - SIMBOLO NAO RECONHECIDO!\n", yytext);

"//".*          /* nada */
"/*"            BEGIN(comment);
<comment>"*/"   BEGIN(INITIAL);
<comment>.      /* nada */
<comment>\n     ++yylineno;

%%

int yywrap(void) { return 1; }