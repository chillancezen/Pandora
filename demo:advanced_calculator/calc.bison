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
    struct param_node * paramnode;
    struct schema_node *schemanode;
    struct statement_node * statementnode;
}

%type <astnode> expression
%type <paramnode> param_list
%type <schemanode> schema_list
%type <statementnode> statement 
%type <statementnode> statement_list
%type <statementnode> conditional_statement
%type <statementnode> loop_statement

%start top_statement

%%

top_statement: /*could be nothing*/
    | top_statement DEFINE IDENTIFER LPARENTHESES schema_list RPARENTHESES LBRACKET statement_list RBRACKET {
        printf("define function: %s\n", $3);
      };
    | top_statement statement
    ;

schema_list: /* could be null parameter list*/ {
        $$ = new_schema_node(SCHEMA_NODE_TYPE_VOID, NULL, NULL);
      }
    | VOID {
        $$ = new_schema_node(SCHEMA_NODE_TYPE_VOID, NULL, NULL);
      }
    | IDENTIFER {
        $$ = new_schema_node(SCHEMA_NODE_TYPE_ID, $1, NULL);
      }
    | schema_list COMMA IDENTIFER {
        $$ = new_schema_node(SCHEMA_NODE_TYPE_ID, $3, $1);
      }
    ;

statement_list: /*also null statement list is allowed*/ {
        $$ = new_statement_node(STATEMENT_TYPE_VOID, NULL);
      }
    | statement_list statement {
        // note the statement is always pointing the rear. 
        concat_statement_nodes($1, $2);
        $$ = $2;
      }
    ;


statement: SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_VOID, NULL);
      }
    | DECLARE IDENTIFER SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_DECLARE, NULL, $2, NULL);
      }
    | DECLARE IDENTIFER ASSIGN expression SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_DECLARE, NULL, $2, $4);
      }
    | IDENTIFER ASSIGN expression SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_ASSIGN, NULL, $1, $3);
      }
    | RETURN SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_RETURN, NULL, NULL);
      }
    | RETURN expression SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_RETURN, NULL, $2);
      }
    | conditional_statement {
        $$ = $1;
      }
    | loop_statement {
        $$ = $1;
      }
    | expression SEMICOLON {
        $$ = new_statement_node(STATEMENT_TYPE_RAW_EXPRESSION, NULL, $1);
      }
    ;

conditional_statement:
      IF LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET {
        $$ = new_statement_node(STATEMENT_TYPE_CONDITIONAL_EXPRESSION, NULL, $3, statement_list_head($6), NULL);
      }
    | IF LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET ELSE LBRACKET statement_list RBRACKET {
        $$ = new_statement_node(STATEMENT_TYPE_CONDITIONAL_EXPRESSION, NULL, $3,
                                statement_list_head($6),
                                statement_list_head($10));
      }
    ;

loop_statement:
      WHILE LPARENTHESES expression RPARENTHESES LBRACKET statement_list RBRACKET {
          $$ = new_statement_node(STATEMENT_TYPE_LOOP_EXPRESSION, NULL, $3, statement_list_head($6));
      }
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
        $$ = new_ast_node(AST_NODE_TYPE_FUNCTION, $1, NULL, NULL, param_list_head($3));
      }
    ;

 ;
param_list: {
        $$ = new_param_node(PARAM_NODE_TYPE_VOID, NULL, NULL);
      }
    | expression {
        $$ = new_param_node(PARAM_NODE_TYPE_AST, $1, NULL);
      }
    | param_list COMMA expression {
        $$ = new_param_node(PARAM_NODE_TYPE_AST, $3, $1);
      }
    ;
%%

int
main(int argc, char **argv)
{

    yyparse();
    return 0;
}
