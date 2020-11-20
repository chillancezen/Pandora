all:
	@flex --outfile=main.flex.c --yylineno main.flex
	@gcc main.flex.c entry.c nested.c -I. -lfl -o flec

