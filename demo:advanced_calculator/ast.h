#ifndef _AST_H
#define _AST_H
#include <stdint.h>

enum ast_node_type {
    AST_NODE_TYPE_ATOM_ID = 1,
    AST_NODE_TYPE_ATOM_INSTANT,
    AST_NODE_TYPE_ATOM_OPS_ADD,
    AST_NODE_TYPE_ATOM_OPS_SUB,
    AST_NODE_TYPE_ATOM_OPS_MUL,
    AST_NODE_TYPE_ATOM_OPS_DIV,
    AST_NODE_TYPE_ATOM_OPS_EQUAL,
    AST_NODE_TYPE_ATOM_OPS_NOTEQUAL,
    AST_NODE_TYPE_ATOM_OPS_GREATERTHAN,
    AST_NODE_TYPE_ATOM_OPS_LESSTHAN,
    AST_NODE_TYPE_ATOM_OPS_AND,
    AST_NODE_TYPE_ATOM_OPS_OR,
    AST_NODE_TYPE_FUNCTION,
};

struct ast_node {
    enum ast_node_type node_type;

    union {
        // AST_NODE_TYPE_ATOM_INSTANT
        int ivalue;

        //AST_NODE_TYPE_ATOM_ID
        // the string is duplicated from lexer, once our ast store them, we are to free them later.
        char * svalue;

        // AST_NODE_TYPE_FUNCTION
        struct {
            char * functionname;
            struct param_node * param_list;
        };
    };

    struct ast_node * lchild;
    struct ast_node * rchild;

    // Debug Info
    int lineno;
};


struct ast_node *
new_ast_node(enum ast_node_type type,
             void * node_value,
             struct ast_node * lchild,
             struct ast_node * rchild,
             ...);




enum param_node_type {
    PARAM_NODE_TYPE_VOID = 1,
    PARAM_NODE_TYPE_AST
};

struct param_node {
    enum param_node_type node_type;
    struct ast_node * ast;
    struct param_node * next_param;
    struct param_node * prev_param;

    int lineno;
};

struct param_node *
new_param_node(enum param_node_type type,
               struct ast_node * ast,
               struct param_node * prev);

struct param_node *
param_list_head(struct param_node * node);


enum statement_node_type {
    STATEMENT_TYPE_VOID = 1,
    STATEMENT_TYPE_DECLARE,
    STATEMENT_TYPE_ASSIGN,
    STATEMENT_TYPE_RETURN,
    STATEMENT_TYPE_RAW_EXPRESSION,
    STATEMENT_TYPE_CONDITIONAL_EXPRESSION,
    STATEMENT_TYPE_LOOP_EXPRESSION,
    STATEMENT_TYPE_CONTINUE,
    STATEMENT_TYPE_BREAK
};

struct statement_node {
    enum statement_node_type node_type;
 
    union {
        // STATEMENT_TYPE_DECLARE
        // STATEMENT_TYPE_ASSIGN
        struct {
            char * variable_name;
            union {
                struct ast_node * initializer;
                struct ast_node * rvalue;
            };
        };
        // STATEMENT_TYPE_RETURN
        struct ast_node * return_value;
        // STATEMENT_TYPE_RAW_EXPRESSION
        struct ast_node * raw_expression;

        // STATEMENT_TYPE_CONDITIONAL_EXPRESSION
        struct {
            struct ast_node * conditional_expression;
            struct statement_node * hit_statements;
            struct statement_node * non_hit_statements;
        };
        // STATEMENT_TYPE_LOOP_EXPRESSION
        struct {
            struct ast_node * loop_expression;
            struct statement_node * body_statements;
        };
    };
    struct statement_node * next_statement;
    struct statement_node * prev_statement;

    int lineno;
};

struct statement_node *
new_statement_node(enum statement_node_type type,
                   struct statement_node * prev,
                   ...);

void
concat_statement_nodes(struct statement_node * pnode,
                       struct statement_node * nnode);

struct statement_node *
statement_list_head(struct statement_node * node);


enum schema_node_type {
    SCHEMA_NODE_TYPE_VOID = 1,
    SCHEMA_NODE_TYPE_ID
};


struct schema_node {
    enum schema_node_type node_type;
    char * variable_name;
    struct schema_node * prev_node;
    struct schema_node * next_node;

    int lineno;
};

struct schema_node *
new_schema_node(enum schema_node_type type,
                char * variable_name,
                struct schema_node * prev);


struct schema_node *
schema_list_head(struct schema_node * node);


//////////////////////////////////////////////
//Global Data Structure
/////////////////////////////////////////////


struct function_declaration {
    char * function_name;
    struct schema_node * parameter_schema;
    struct statement_node * body;
    struct function_declaration *next_function;

    int lineno;
};

struct function_declaration *
new_function_declaration(char * function_name,
                         struct schema_node * params_list,
                         struct statement_node * body);

void
register_function(struct function_declaration * function);

void
register_global_statement(struct statement_node * statement);

#endif
