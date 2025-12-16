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

extern void _yyerror(int,char*);
extern int yylineno;
extern elemTabSimb tabSimb[];

static int n_variaveis = 0;
static int n_parametros = 0;
static int tipo;
static int n_rotulos = 1;
static int aux;

static listaPars **listaParametros;
static listaPars *parametro;

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

void desalocarArvore(ptno raiz){
    
    if(!raiz) return;

    for(ptno n = raiz->filho; n; n = n->irmao)
        desalocarArvore(n);

    if(raiz->id) free(raiz->id);
    free(raiz);
}

// --------------------- GERAÇÃO DE CÓDIGO -------------------------

static void idNaoEncontrado(ptno p){
    char msg[200];
    sprintf(msg, "\"%s\" não foi definido", p->id);
    _yyerror(p->linha, msg);
}

static void idDuplicado(ptno p){
    char msg[200];
    sprintf(msg, "Dupla definição de \"%s\"", p->id);
    _yyerror(p->linha, msg);
}

// "p" deve ser o identificador que referencia o local onde 
// o valor deve ser armazenado
static void armazenar(FILE *arq, ptno p, char* erroTipo, char* erroProc){

    int s = buscaSimbolo(p->id);

    if(s < 0)
        idNaoEncontrado(p);

    // Se "erroTipo" for NULL, pular a verificação de tipo
    // Isso é feito para que nenhuma verificação seja feita em um comando de leitura
    if(erroTipo && tabSimb[s].tip != p->irmao->valor)
        _yyerror(p->linha, erroTipo);

    switch(tabSimb[s].cat)
    {
        case VAR: fprintf(arq, "\tARZG\t%d\n", tabSimb[s].dsl); break;
        case FUN: fprintf(arq, "\tARZL\t%d\n", tabSimb[s].dsl); break;

        case PAR:
            switch(tabSimb[s].mec)
            {
                case VAL: fprintf(arq, "\tARZL\t%d\n", tabSimb[s].dsl); break;
                case REF: fprintf(arq, "\tARMI\t%d\n", tabSimb[s].dsl); break;
            }
        break;

        case PRO: _yyerror(p->linha, erroProc); break;
    }
}

static void empilharArgumentos(FILE* arq, ptno p, int s, char *erroPoucosArgs){

    // Recupera o nó do primeiro parâmetro da rotina
    // Os demais nós podem ser recuperados por encadeamento a partir deste
    parametro = tabSimb[s].par;

    // Carrega argumentos na pilha
    geraCodigo(arq, p->filho->irmao);

    // Caso um ou mais parâmetros não sejam pareados com argumentos na chamada
    if(parametro)
        _yyerror(p->linha, erroPoucosArgs);

    fprintf(arq, "\tSVCP\n");
    fprintf(arq, "\tDSVS\tL%d\n", tabSimb[s].rot);
}
   
void geraCodigo(FILE *arq, ptno p){

    if(!p) return;

    ptno p_i = p->filho;
    int simbolo;

    switch(p->tipo){

        case PROGRAMA:
        
            fprintf(arq, "\tINPP\n");

            for(p_i = p_i->irmao; p_i; p_i = p_i->irmao)
            {
                switch(p_i->tipo)
                {
                    case LISTA_ROTINAS:
                        fprintf(arq, "\tDSVS\tL0\n");
                    break;

                    case LISTA_COMANDOS:
                        fprintf(arq, "L0\tNADA\n");
                    break;
                }

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

            if(0 > inserirSimbolo(
                    criarSimbolo(
                        p_i->id,                // Identificador
                        GLOBAL,                 // Escopo
                        n_variaveis++,          // Deslocamento
                        VAZIO,                  // Rótulo       
                        VAR,                    // Categoria
                        tipo,                   // Tipo
                        VAZIO                   // Mecanismo
                    )
                )   
            ) idDuplicado(p_i);
             
            if(p_i->irmao) geraCodigo(arq, p_i->irmao);
            else if(n_variaveis > 0) fprintf(arq, "\tAMEM\t%d\n", n_variaveis);

        break;

        case LISTA_ROTINAS:

            n_parametros = 0;

            // Gera código da rotina atual
            geraCodigo(arq, p_i);

            // Remove da tabela os parâmetros da rotina
            removerSimbolos(n_parametros);

            // Gera código das demais rotinas
            geraCodigo(arq, p_i->irmao);

        break;

        case FUNCAO:

            simbolo = inserirSimbolo(
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

            if(simbolo < 0)
                idDuplicado(p);
            
            fprintf(arq, "L%d\tENSP\n", n_rotulos++);

            // Recupera o ponteiro para a lista de parâmetros (então vazia)
            // A lista será preenchida conforme for feita a passagem pela declaração dos parâmetros
            listaParametros = recuperarLista(simbolo);

            // Passagem pelos parâmetros da função
            geraCodigo(arq, p_i->irmao);

            // Após a passagem pela lista de parâmetros, é possível saber o endereço
            // onde deve ser armazenado o retorno da função
            atualizarDeslocamento(simbolo, -(3 + n_parametros));
            
            // Gera código do corpo da função (comandos)
            geraCodigo(arq, p_i->irmao->irmao);

            // Retorna para o chamador
            fprintf(arq, "\tRTSP\t%d\n", n_parametros);

        break;

        case PROCEDIMENTO:
            
            simbolo = inserirSimbolo(
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

            if(simbolo < 0) 
                idDuplicado(p);
            
            fprintf(arq, "L%d\tENSP\n", n_rotulos++);

            // Recupera o ponteiro para a lista de parâmetros (então vazia)
            // A lista será preenchida conforme for feita a passagem pela declaração dos parâmetros
            listaParametros = recuperarLista(simbolo);

            // Passagem pelos parâmetros
            geraCodigo(arq, p_i);

            // Gera código do corpo do procedimento (comandos)
            geraCodigo(arq, p_i->irmao);

            // Retorna para o chamador
            fprintf(arq, "\tRTSP\t%d\n", n_parametros);

        break;

        case LISTA_PARAMETROS:
            // Por questão de eficiência, a função de inserir parâmetros na lista
            // o faz de trás para frente, o que torna necessário visitar os parâmetros
            // ao contrário na árvore
            geraCodigo(arq, p_i->irmao);
            geraCodigo(arq, p_i);
        break;

        case PARAMETRO:

            if(0 > inserirSimbolo(
                    criarSimbolo(
                        p_i->irmao->irmao->id,  // Identificador
                        LOCAL,                  // Escopo
                        -(3 + n_parametros++),  // Deslocamento
                        VAZIO,                  // Rótulo       
                        PAR,                    // Categoria
                        p_i->irmao->valor,      // Tipo
                        p_i->valor              // Mecanismo
                    )
                )
            ) idDuplicado(p_i->irmao->irmao);

            inserirPar(listaParametros, criarPar(p_i->irmao->valor, p_i->valor));

        break;

        case CHAMADA_FUNCAO:

            fprintf(arq, "\tAMEM\t1\n");

            // Busca o identificador da função e verifica se já foi definido
            simbolo = buscaSimbolo(p_i->id);

            if(simbolo < 0)
                idNaoEncontrado(p);

            // Recupera o tipo de retorno da função e o armazena no nó
            p->valor = tabSimb[simbolo].tip;

            empilharArgumentos(arq, p, simbolo, "Função chamada com poucos argumentos");

        break;

        case CHAMADA_PROCEDIMENTO:

            // Busca o identificador do procedimento e verifica se já foi definido
            simbolo = buscaSimbolo(p_i->id);

            if(simbolo < 0)
                idNaoEncontrado(p_i);

            empilharArgumentos(arq, p, simbolo, "Procedimento chamado com poucos argumentos");

        break;

        case LISTA_ARGUMENTOS:

            // Caso os parâmetros da rotina já tenham se esgotado mas ainda haja argumentos,
            // gera erro de compilação.
            if(!parametro)
                _yyerror(p_i->linha, "Argumentos em excesso na chamada de rotina");

            switch(parametro->mec)
            {
                case VAL:

                    // Caso a passagem seja por valor, gera o código para avaliar a expressão
                    // passada como argumento 
                    geraCodigo(arq, p_i);

                    // Verifica se o tipo da expressão é compatível com o do parâmetro
                    if(parametro->tip != p_i->valor)
                        _yyerror(p_i->linha, "Tipo do argumento incompatível com tipo do parâmetro");

                break;

                // Caso a passagem seja por referência, NÃO FAZ chamada recursiva
                // Passagens por referência exigem que o argumento seja um identificador válido
                case REF:

                    // Busca o identificador na tabela de símbolos
                    simbolo = buscaSimbolo(p_i->id);

                    // Verifica se ele foi encontrado, gera erro de compilação caso não tenha sido
                    if(simbolo < 0)
                        idNaoEncontrado(p_i);

                    // Verifica se o tipo do identificador passado como argumento
                    // é compatível com o parâmetro esperado e encerra o programa caso não seja
                    if(parametro->tip != tabSimb[simbolo].tip)
                        _yyerror(p_i->linha, "Tipo do argumento incompatível com parâmetro");

                    // Verifica de que forma o endereço deve ser obtido para ser empilhado
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
                            _yyerror(p_i->linha, 
                                "Procedimentos não podem ser passados como argumentos de rotinas"
                            );
                        break;
                    }

                break;
            }
            
            // Avança para o próximo parâmetro da rotina
            parametro = parametro->prox;

            geraCodigo(arq, p_i->irmao);

        break;

        case LISTA_COMANDOS:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);
        break;

        case LEITURA:
            // Comando de leitura
            fprintf(arq, "\tLEIA\n");

            // Armazena topo da pilha, de acordo com a categoria do identificador
            armazenar(arq, p_i, NULL, "Retornos de leituras não podem ser passadas para procedimentos");
        break;

        case ESCRITA:
            // Avalia a expressão cujo resultado deve ser escrito
            geraCodigo(arq, p_i);

            // Comando de escrita
            fprintf(arq, "\tESCR\n");
        break;

        case REPETICAO:
            // Rótulo para a avaliação da expressão lógica
            fprintf(arq, "L%d\tNADA\n", empilha(n_rotulos++));
            geraCodigo(arq, p_i);

            // Se o retorno da expressão não for lógica, gera erro de compilação.
            if(p_i->valor != LOG)
                _yyerror(p_i->linha, "Expressão de comando de repetição precisa ter valor lógico");

            // Sai do laço caso a expressão seja avaliada como falsa
            fprintf(arq, "\tDSVF\tL%d\n", empilha(n_rotulos++));

            // Gera o código do corpo do laço
            geraCodigo(arq, p_i->irmao);

            // Código para retornar à avaliação da expressão lógica
            aux = desempilha();
            fprintf(arq, "\tDSVS\tL%d\n", desempilha());

            // Rótulo para a saída do laço
            fprintf(arq, "L%d\tNADA\n", aux);
        break;

        case SELECAO:
            // Gera código para avaliar a expressão lógica
            geraCodigo(arq, p_i);

            // Se o retorno da expressão não for lógica, gera erro de compilação.
            if(p_i->valor != LOG)
                _yyerror(p_i->linha, "Expressão de comando de seleção precisa ter valor lógico");

            // Se a expressão for avaliada como falsa, vá para o código do "senão",
            // ou, caso este não exista, para o fim do bloco
            fprintf(arq, "\tDSVF\tL%d\n", empilha(n_rotulos++));

            // Gera código do "então"
            geraCodigo(arq, p_i->irmao); 

            // Gera código do "senão", caso ele exista
            if(p_i->irmao->irmao){
                aux = desempilha();
                fprintf(arq, "\tDSVS\tL%d\n", empilha(n_rotulos++));
                fprintf(arq, "L%d\tNADA\n", aux);

                geraCodigo(arq, p_i->irmao->irmao);
            }

            // Rótulo marcando o fim do comando de seleção
            fprintf(arq, "L%d\tNADA\n", desempilha());
        break;

        case ATRIBUICAO:
            // Gera código da avaliação da expressão do lado direito da atribuição
            geraCodigo(arq, p_i->irmao);

            // Armazena resultado da expressão no identificador de acordo com sua categoria
            // Gera erro de compilação caso o tipo do identificador seja diferente do tipo
            // da expressão ou caso o identificador do lado esquerdo da atribuição seja
            // de um procedimento 
            armazenar(arq, p_i, 
                "Comando de atribuição precisa ter tipos compatíveis",
                "Procedimentos não podem ser utilizados do lado esquerdo de atribuições"
            );
        break;

        case MULTIPLICACAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Somente valores inteiros podem ser multiplicados");
            
            fprintf(arq, "\tMULT\n");
        break;

        case DIVISAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Somente valores inteiros podem ser divididos");
            
            fprintf(arq, "\tDIVI\n");
        break;

        case SOMA:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Somente valores inteiros podem ser somados");
            
            fprintf(arq, "\tSOMA\n");

        break;
        
        case SUBTRACAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Somente valores inteiros podem ser subtraídos");
            
            fprintf(arq, "\tSUBT\n");
        break;

        case COMPARA_MAIOR:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Operador de maior precisa de dois valores inteiros");

            fprintf(arq, "\tCMMA\n");
        break;

        case COMPARA_MENOR:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != INT || p_i->irmao->valor != INT)
                _yyerror(p->linha, "Operador de menor precisa de dois valores inteiros");

            fprintf(arq, "\tCMME\n");
        break;

        case COMPARA_IGUAL:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != p_i->irmao->valor)
                _yyerror(p->linha, "Operador de igualdade precisa ter valores de mesmo tipo");

            fprintf(arq, "\tCMIG\n");
        break;

        case CONJUNCAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != LOG || p_i->irmao->valor != LOG)
                _yyerror(p->linha, "Operador de conjunção precisa de valores lógicos");
            
            fprintf(arq, "\tCONJ\n");
        break;

        case DISJUNCAO:
            geraCodigo(arq, p_i);
            geraCodigo(arq, p_i->irmao);

            if(p_i->valor != LOG || p_i->irmao->valor != LOG)
                _yyerror(p->linha, "Operador de disjunção precisa de valores lógicos");
            
            fprintf(arq, "\tDISJ\n");
        break;

        case NEGACAO:
            geraCodigo(arq, p_i);

            if(p_i->valor != LOG)
                _yyerror(p_i->linha, "Somente valores lógicos podem ser negados");

            fprintf(arq, "\tNEGA\n");
        break;

        case NUMERO:
        case LOGICO:
            fprintf(arq, "\tCRCT\t%s\n", p->id);
        break;

        case IDENTIFICADOR:

            simbolo = buscaSimbolo(p->id);

            if(simbolo < 0)
                idNaoEncontrado(p);
            
            switch(tabSimb[simbolo].cat)
            {
                case VAR: fprintf(arq, "\tCRVG\t%d\n", tabSimb[simbolo].dsl); break;
                case FUN: fprintf(arq, "\tCRVL\t%d\n", tabSimb[simbolo].dsl); break;

                case PAR:
                    switch(tabSimb[simbolo].mec)
                    {
                        case VAL: fprintf(arq, "\tCRVL\t%d\n", tabSimb[simbolo].dsl); break;
                        case REF: fprintf(arq, "\tCRVI\t%d\n", tabSimb[simbolo].dsl); break;
                    }
                break;

                case PRO: _yyerror(p->linha, 
                    "Procedimentos não podem ser utilizados em expressões"); break;
            }

        p->valor = tabSimb[simbolo].tip;

        break;
    }
}