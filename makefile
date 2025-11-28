COMPILADOR = simples

ANALISADORES = analisadores
FONTE = codigos_c
OBJ = codigos_o

NOMES = utils lexico sintatico tree
NOMES_OBJ = $(addprefix $(OBJ)/, $(addsuffix .o, $(NOMES)))

all: $(COMPILADOR) mvs

# EXECUTÁVEIS

$(COMPILADOR): $(NOMES_OBJ)
	gcc $^ -o $@

mvs: $(OBJ)/mvs.o $(OBJ)
	gcc $< -o $@

$(OBJ):
	mkdir $@

# CÓDIGOS .c e .h

$(FONTE)/lexico.c: $(ANALISADORES)/lexico.l $(OBJ)
	lex -o $@ $<  

$(FONTE)/sintatico.h $(FONTE)/sintatico.c: $(ANALISADORES)/sintatico.y $(OBJ)
	bison -d $< -o $@ 

# CÓDIGOS .o

$(OBJ)/lexico.o: $(FONTE)/lexico.c $(FONTE)/sintatico.h $(OBJ)
	gcc $< -c -o $@

$(OBJ)/%.o: $(FONTE)/%.c $(OBJ)
	gcc $< -c -o $@

# LIMPAR

clean:
	rm -rf $(OBJ)
	rm -f $(FONTE)/sintatico.c $(FONTE)/sintatico.h $(FONTE)/lexico.c
	rm -f $(COMPILADOR) mvs *.dot *.svg *.mvs