%{
    #include <stdio.h>
    #include <ast.h>

    extern int yylex(void);
    extern int yyerror(const char *);
%}

%token DEFINE // define a funtion
%token DECLARE // declare a variable

%token <sval> IDENTIFER
%token <ival> NUMBER
%token ADD SUB MUL DIV
%token EOL
%token LPARENTHESES RPARENTHESES
%token LBRACKET RBRACKET
%token ASSIGN
%token COMMA
%token VOID
%token RETURN
%token IF ELSE WHILE
%token SEMICOLON

%left OR
%left AND
%left EQUAL NOTEQUAL GREATERTHAN LESSTHAN
%left ADD SUB
%left MUL DIV

%token EQUAL NOTEQUAL GREATERTHAN LESSTHAN
%token OR AND

%union {
    int ival;
    char * sval;
}

%union {
    struct ast_node * astnode;
}

%type <astnode> expression

%start top_statement

%%

top_statement: /*could be nothing*/
    | top_statement DEFINE IDENTIFER LPARENTHESES schema_list RPARENTHESES LBRACKET statement_list RBRACKET {
        printf("define function: %s\n", $3);
      };
    | top_statement statement
    ;

schema_list: /* could be null parameter list*/
    | VOID
    | IDENTIFER
    | schema_list COMMA IDENTIFER
    ;

statement_list: /*also null statement list is allowed*/
    | statement_list statement
    ;


statement: SEMICOLON
    |DECLARE IDENTIFER SEMICOLON
    |DECLARE IDENTIFER ASSIGN expression SEMICOLON
    |IDENTIFER ASSIGN expression SEMICOLON
    |RETURN expression SEMICOLON
    |conditional_expression
    |loop_expression
    ;

conditional_expression:
      IF LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET
    | IF LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET ELSE LBRACKET statement_list RBRACKET
    ;

loop_expression:
    WHILE LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET
    ;

expression:
      IDENTIFER {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_ID, $1, NULL, NULL);
      }
    | NUMBER {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_INSTANT, (void *)(uint64_t)$1, NULL, NULL); 
      }
    | LPARENTHESES expression RPARENTHESES {
        $$ = $2;
      }
    | expression ADD expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_ADD, NULL, $1, $3);
      }
    | expression SUB expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_SUB, NULL, $1, $3);
      }
    | expression MUL expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_MUL, NULL, $1, $3);
      }
    | expression DIV expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_DIV, NULL, $1, $3);
      }
    | expression EQUAL expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_EQUAL, NULL, $1, $3);
      }
    | expression NOTEQUAL expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_NOTEQUAL, NULL, $1, $3);
      }
    | expression GREATERTHAN expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_GREATERTHAN, NULL, $1, $3);
      }
    | expression LESSTHAN expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_DIV, NULL, $1, $3);
      }
    | expression AND expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_AND, NULL, $1, $3);
      }
    | expression OR expression {
        $$ = new_ast_node(AST_NODE_TYPE_ATOM_OPS_OR, NULL, $1, $3);
      }
    | IDENTIFER LPARENTHESES param_list RPARENTHESES {
        $$ = NULL;
      }
    ;

param_list:
    | IDENTIFER
    | NUMBER
    | param_list COMMA IDENTIFER
    | param_list COMMA NUMBER
    ;
 
%%

int
main(int argc, char **argv)
{

    yyparse();
    return 0;
}
