%{
    #include <stdio.h>

    extern int yylex(void);
    extern int yyerror(const char *);
%}

%token NUMBER
%token ADD SUB MUL DIV
%token EOL
%token LPARATHESES RPARATHESES

%%

top_statement:
    | top_statement add_statement EOL { printf("=%d\n", $2); }
    ;

add_statement: mul_statement
    | add_statement ADD mul_statement { $$ = $1 + $3; }
    | add_statement SUB mul_statement { $$ = $1 - $3; }
    ;

mul_statement: term
    | mul_statement MUL term { $$ = $1 * $3; }
    | mul_statement DIV term { $$ = $1 / $3; }
    ;

term: NUMBER
    | LPARATHESES add_statement RPARATHESES { $$ = $2; }
    | SUB NUMBER { $$ = -$2; }
    | ADD NUMBER { $$ = $2; }
    ;

%%

int
main(int argc, char **argv)
{

    yyparse();
    return 0;
}
