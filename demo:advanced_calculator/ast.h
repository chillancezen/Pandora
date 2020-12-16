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
    AST_NODE_TYPE_ATOM_OPS_OR
};

struct ast_node {
    enum ast_node_type node_type;

    union {
        // AST_NODE_TYPE_ATOM_INSTANT
        int ivalue;

        //AST_NODE_TYPE_ATOM_ID
        // the string is duplicated from lexer, once our ast store them, we are to free them later.
        char * svalue;
    };

    struct ast_node * lchild;
    struct ast_node * rchild;
};


struct ast_node *
new_ast_node(enum ast_node_type type,
             void * node_value,
             struct ast_node * lchild,
             struct ast_node * rchild);
#endif
