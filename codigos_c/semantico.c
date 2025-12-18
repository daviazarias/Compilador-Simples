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
#include "semantico.h"
#include "simbolos.h"
#include "pilha.h"

#define P1 p->filho
#define P2 P1->irmao
#define P3 P2->irmao

#define PARAMS FILE* arq, ptno p
#define NADA nada

#define OPERADOR_BINARIO(inst,tip,err)          \
    geraCodigo(arq, P1);                        \
    geraCodigo(arq, P2);                        \
    if(P1->valor != tip || P2->valor != tip)    \
        _yyerror(p->linha, err);                \
    fprintf(arq, "\t" inst "\n"); 

static int n_variaveis = 0;
static int n_parametros = 0;
static int n_rotulos = 1;
static int tipo;

static listaPars **listaParametros = NULL;
static listaPars *parametro = NULL;

extern elemTabSimb tabSimb[];
extern void _yyerror(int,char*);

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

    // Empilha lista de parâmetros e ponteiro de parâmetro atuais, no caso
    // de um dos argumentos da rotina for uma função
    empilhaPtr((void*) parametro);
    empilhaPtr((void*) listaParametros);

    // Recupera o nó do primeiro parâmetro da rotina
    // Os demais nós podem ser recuperados por encadeamento a partir deste
    parametro = tabSimb[s].par;

    // Carrega argumentos na pilha
    geraCodigo(arq, p->filho->irmao);

    // Caso um ou mais parâmetros não sejam pareados com argumentos na chamada
    if(parametro)
        _yyerror(p->linha, erroPoucosArgs);

    // Desempilha a lista de parâmetros e ponteiro de parâmetros empilhados anteriormente
    listaParametros = (listaPars**) desempilha().ponteiro;
    parametro = (listaPars*) desempilha().ponteiro;

    fprintf(arq, "\tSVCP\n");
    fprintf(arq, "\tDSVS\tL%d\n", tabSimb[s].rot);
}

static void gerarPrograma(PARAMS){

    fprintf(arq, "\tINPP\n");

    for(ptno p_i = P1->irmao; p_i; p_i = p_i->irmao)
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
}

static void gerarDeclaracaoVariaveis(PARAMS){

    tipo = P1->valor;
    geraCodigo(arq, P2);
    geraCodigo(arq, P3);
}

static void gerarListaVariaveis(PARAMS){
    if(0 > inserirSimbolo(
            criarSimbolo(
                P1->id,                // Identificador
                GLOBAL,                 // Escopo
                n_variaveis++,          // Deslocamento
                VAZIO,                  // Rótulo       
                VAR,                    // Categoria
                tipo,                   // Tipo
                VAZIO                   // Mecanismo
            )
        )   
    ) idDuplicado(P1);
        
    if(P1->irmao) geraCodigo(arq, P2);
    else fprintf(arq, "\tAMEM\t%d\n", n_variaveis);
}

static void gerarListaRotinas(PARAMS){
    n_parametros = 0;

    // Gera código da rotina atual
    geraCodigo(arq, P1);

    // Remove da tabela os parâmetros da rotina
    removerSimbolos(n_parametros);

    // Gera código das demais rotinas
    geraCodigo(arq, P2);
}

static void gerarDeclaracaoFuncao(PARAMS){
    int simbolo = inserirSimbolo(
            criarSimbolo(
                p->id,                  // Identificador
                GLOBAL,                 // Escopo
                VAZIO,                  // Deslocamento -- DESCONHECIDO
                n_rotulos,              // Rótulo       
                FUN,                    // Categoria
                P1->valor,             // Tipo
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
    geraCodigo(arq, P2);

    // Após a passagem pela lista de parâmetros, é possível saber o endereço
    // onde deve ser armazenado o retorno da função
    atualizarDeslocamento(simbolo, -(3 + n_parametros));
    
    // Gera código do corpo da função (comandos)
    geraCodigo(arq, P3);

    // Retorna para o chamador
    fprintf(arq, "\tRTSP\t%d\n", n_parametros);
}

static void gerarDeclaracaoProcedimento(PARAMS){
    int simbolo = inserirSimbolo(
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
    geraCodigo(arq, P1);

    // Gera código do corpo do procedimento (comandos)
    geraCodigo(arq, P2);

    // Retorna para o chamador
    fprintf(arq, "\tRTSP\t%d\n", n_parametros);
}

static void gerarListaParametros(PARAMS){
    // Por questão de eficiência, a função de inserir parâmetros na lista
    // o faz de trás para frente, o que torna necessário visitar os parâmetros
    // ao contrário na árvore
    geraCodigo(arq, P2);
    geraCodigo(arq, P1);
}

static void gerarParametro(PARAMS){

    if(0 > inserirSimbolo(
            criarSimbolo(
                P1->irmao->irmao->id,  // Identificador
                LOCAL,                  // Escopo
                -(3 + n_parametros++),  // Deslocamento
                VAZIO,                  // Rótulo       
                PAR,                    // Categoria
                P1->irmao->valor,      // Tipo
                P1->valor              // Mecanismo
            )
        )
    ) idDuplicado(P3);

    inserirPar(listaParametros, criarPar(P2->valor, P1->valor));
}

static void gerarChamadaFuncao(PARAMS){
    // Busca o identificador da função e verifica se já foi definido
    int simbolo = buscaSimbolo(P1->id);

    if(simbolo < 0)
        idNaoEncontrado(p);

    fprintf(arq, "\tAMEM\t1\n");

    // Recupera o tipo de retorno da função e o armazena no nó
    p->valor = tabSimb[simbolo].tip;

    empilharArgumentos(arq, p, simbolo, "Função chamada com poucos argumentos");
}

static void gerarChamadaProcedimento(PARAMS){
    // Busca o identificador do procedimento e verifica se já foi definido
    int simbolo = buscaSimbolo(P1->id);

    if(simbolo < 0)
        idNaoEncontrado(P1);

    empilharArgumentos(arq, p, simbolo, "Procedimento chamado com poucos argumentos");
}

static void gerarListaArgumentos(PARAMS){

    int simbolo;

    // Caso os parâmetros da rotina já tenham se esgotado mas ainda haja argumentos,
    // gera erro de compilação.
    if(!parametro)
        _yyerror(P1->linha, "Argumentos em excesso na chamada de rotina");

    switch(parametro->mec)
    {
        case VAL:

            // Caso a passagem seja por valor, gera o código para avaliar a expressão
            // passada como argumento 
            geraCodigo(arq, P1);

            // Verifica se o tipo da expressão é compatível com o do parâmetro
            if(parametro->tip != P1->valor)
                _yyerror(P1->linha, "Tipo do argumento incompatível com tipo do parâmetro");
        break;

        // Caso a passagem seja por referência, NÃO FAZ chamada recursiva
        // Passagens por referência exigem que o argumento seja um identificador válido
        case REF:

            // Busca o identificador na tabela de símbolos
            simbolo = buscaSimbolo(P1->id);

            // Verifica se ele foi encontrado, gera erro de compilação caso não tenha sido
            if(simbolo < 0)
                idNaoEncontrado(P1);

            // Verifica se o tipo do identificador passado como argumento
            // é compatível com o parâmetro esperado e encerra o programa caso não seja
            if(parametro->tip != tabSimb[simbolo].tip)
                _yyerror(P1->linha, "Tipo do argumento incompatível com parâmetro");

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
                    _yyerror(P1->linha, 
                        "Procedimentos não podem ser passados como argumentos de rotinas"
                    );
                break;
            }

        break;
    }
    
    // Avança para o próximo parâmetro da rotina
    parametro = parametro->prox;

    geraCodigo(arq, P2);
}

static void gerarListaComandos(PARAMS){
    geraCodigo(arq, P1);
    geraCodigo(arq, P2);
}

static void gerarLeitura(PARAMS){
    // Comando de leitura
    fprintf(arq, "\tLEIA\n");

    // Armazena topo da pilha, de acordo com a categoria do identificador
    armazenar(arq, P1, NULL, "Retornos de leituras não podem ser passadas para procedimentos");
}

static void gerarEscrita(PARAMS){
    // Avalia a expressão cujo resultado deve ser escrito
    geraCodigo(arq, P1);

    // Comando de escrita
    fprintf(arq, "\tESCR\n");
}

static void gerarRepeticao(PARAMS){
    // Rótulo para a avaliação da expressão lógica
    fprintf(arq, "L%d\tNADA\n", empilhaRot(n_rotulos++));
    geraCodigo(arq, P1);

    // Se o retorno da expressão não for lógica, gera erro de compilação.
    if(P1->valor != LOG)
        _yyerror(P1->linha, "Expressão de comando de repetição precisa ter valor lógico");

    // Sai do laço caso a expressão seja avaliada como falsa
    fprintf(arq, "\tDSVF\tL%d\n", empilhaRot(n_rotulos++));

    // Gera o código do corpo do laço
    geraCodigo(arq, P2);

    // Código para retornar à avaliação da expressão lógica
    int aux = desempilha().rotulo;
    fprintf(arq, "\tDSVS\tL%d\n", desempilha().rotulo);

    // Rótulo para a saída do laço
    fprintf(arq, "L%d\tNADA\n", aux);
}

static void gerarSelecao(PARAMS){
    // Gera código para avaliar a expressão lógica
    geraCodigo(arq, P1);

    // Se o retorno da expressão não for lógica, gera erro de compilação.
    if(P1->valor != LOG)
        _yyerror(P1->linha, "Expressão de comando de seleção precisa ter valor lógico");

    // Se a expressão for avaliada como falsa, vá para o código do "senão",
    // ou, caso este não exista, para o fim do bloco
    fprintf(arq, "\tDSVF\tL%d\n", empilhaRot(n_rotulos++));

    // Gera código do "então"
    geraCodigo(arq, P2); 

    // Gera código do "senão", caso ele exista
    if(P3){
        int aux = desempilha().rotulo;
        fprintf(arq, "\tDSVS\tL%d\n", empilhaRot(n_rotulos++));
        fprintf(arq, "L%d\tNADA\n", aux);

        geraCodigo(arq, P3);
    }

    // Rótulo marcando o fim do comando de seleção
    fprintf(arq, "L%d\tNADA\n", desempilha().rotulo);
}

static void gerarAtribuicao(PARAMS){
    // Gera código da avaliação da expressão do lado direito da atribuição
    geraCodigo(arq, P2);

    // Armazena resultado da expressão no identificador de acordo com sua categoria
    // Gera erro de compilação caso o tipo do identificador seja diferente do tipo
    // da expressão ou caso o identificador do lado esquerdo da atribuição seja
    // de um procedimento 
    armazenar(arq, P1, 
        "Comando de atribuição precisa ter tipos compatíveis",
        "Procedimentos não podem ser utilizados do lado esquerdo de atribuições"
    );
}

static void gerarMultiplicacao(PARAMS){
    OPERADOR_BINARIO("MULT", INT, "Somente valores inteiros podem ser multiplicados")
}

static void gerarDivisao(PARAMS){
    OPERADOR_BINARIO("DIVI", INT, "Somente valores inteiros podem ser divididos")
}

static void gerarSoma(PARAMS){
    OPERADOR_BINARIO("SOMA", INT, "Somente valores inteiros podem ser somados")
}

static void gerarSubtracao(PARAMS){
    OPERADOR_BINARIO("SUBT", INT, "Somente valores inteiros podem ser subtraídos")
}

static void gerarComparaMaior(PARAMS){
    OPERADOR_BINARIO("CMMA", INT, "Operador de maior precisa de dois valores inteiros")
}

static void gerarComparaMenor(PARAMS){
    OPERADOR_BINARIO("CMME", INT, "Operador de menor precisa de dois valores inteiros")
}

static void gerarDisjuncao(PARAMS){
    OPERADOR_BINARIO("DISJ", LOG, "Operador de disjunção precisa de dois valores inteiros")
}

static void gerarConjuncao(PARAMS){
    OPERADOR_BINARIO("CONJ", LOG, "Operador de conjunção precisa de dois valores inteiros")
}

static void gerarComparaIgual(PARAMS){
    geraCodigo(arq, P1);
    geraCodigo(arq, P2);

    if(P1->valor != P2->valor)
        _yyerror(p->linha, "Operador de igualdade precisa ter valores de mesmo tipo");

    fprintf(arq, "\tCMIG\n");
}

static void gerarNegacao(PARAMS){
    geraCodigo(arq, P1);

    if(P1->valor != LOG)
        _yyerror(P1->linha, "Somente valores lógicos podem ser negados");

    fprintf(arq, "\tNEGA\n");
}

static void gerarNumLogico(PARAMS){
    fprintf(arq, "\tCRCT\t%s\n", p->id);
}

static void gerarIdentificador(PARAMS){

    int simbolo = buscaSimbolo(p->id);

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
}

static void nada(PARAMS){}

void (*geracaoCodigo[QTD_TIPOS])(PARAMS) = {
    gerarPrograma,                  // PROGRAMA                 00
    gerarDeclaracaoVariaveis,       // DECLARACAO_VARIAVEIS     01
    gerarListaComandos,             // LISTA_COMANDOS           02
    NADA,                           // TIPO                     03  SEM AÇÃO
    gerarListaVariaveis,            // LISTA_VARIAVEIS          04
    gerarLeitura,                   // LEITURA                  05
    gerarEscrita,                   // ESCRITA                  06
    gerarRepeticao,                 // REPETICAO                07
    gerarSelecao,                   // SELECAO                  08
    gerarAtribuicao,                // ATRIBUICAO               09
    gerarMultiplicacao,             // MULTIPLICACAO            10
    gerarDivisao,                   // DIVISAO                  11
    gerarSoma,                      // SOMA                     12
    gerarSubtracao,                 // SUBTRACAO                13
    gerarComparaMaior,              // COMPARA_MAIOR            14
    gerarComparaMenor,              // COMPARA_MENOR            15
    gerarComparaIgual,              // COMPARA_IGUAL            16
    gerarConjuncao,                 // CONJUNCAO                17
    gerarDisjuncao,                 // DISJUNCAO                18
    gerarNegacao,                   // NEGACAO                  19      
    gerarNumLogico,                 // NUMERO                   20
    gerarNumLogico,                 // LOGICO                   21
    gerarIdentificador,             // IDENTIFICADOR            22
    gerarDeclaracaoFuncao,          // FUNCAO                   23
    gerarDeclaracaoProcedimento,    // PROCEDIMENTO             24
    gerarListaRotinas,              // LISTA_ROTINAS            25
    gerarChamadaFuncao,             // CHAMADA_FUNCAO           26
    gerarChamadaProcedimento,       // CHAMADA_PROCEDIMENTO     27
    gerarListaParametros,           // LISTA_PARAMETROS         28
    gerarListaArgumentos,           // LISTA_ARGUMENTOS         29
    NADA,                           // MECANISMO                30  SEM AÇÃO
    gerarParametro                  // PARAMETRO                31
};

void geraCodigo(PARAMS){
    if(p) geracaoCodigo[p->tipo](arq,p);
}