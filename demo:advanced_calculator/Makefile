all:
	bison --defines=calc.token.h --output=calc.bison.c  calc.bison
	flex --outfile=calc.flex.c --yylineno calc.flex
	gcc -o calc calc.bison.c ast.c semantics.c symbol.c calc.flex.c -lfl -I. -ly
