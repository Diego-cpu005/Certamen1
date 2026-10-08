all: turing

todo: all

SRC = y.tab.c lex.yy.c semantico.c subrutinas.c interprete.c

turing: $(SRC) turing.h
	gcc -Wall -Wno-unused-function $(SRC) -o turing

y.tab.c y.tab.h: sintactico.y turing.h
	bison -dy sintactico.y

lex.yy.c: lexico.l y.tab.h
	flex lexico.l

run: turing
	./turing pruebas/certamen.tm

clean limpiar:
	rm -f lex.yy.c y.tab.c y.tab.h turing

.PHONY: all todo run clean limpiar
