#include <ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <log.h>

#define AST_LOG(...) LOG("line:%d [node:%p lchild:%p rchild:%p] ", yylineno, node, lchild, rchild); LOG(__VA_ARGS__)

extern int yylineno;

struct ast_node *
new_ast_node(enum ast_node_type type,
             void * node_value,
             struct ast_node * lchild,
             struct ast_node * rchild)
{

    struct ast_node * node = (struct ast_node *)malloc(sizeof (struct ast_node));
    assert(node);
    memset(node, 0x0, sizeof(struct ast_node));

    node->node_type = type;
    switch(node->node_type)
    {
        case AST_NODE_TYPE_ATOM_ID:
            node->svalue = (char *)node_value;
            AST_LOG("AST_NODE_TYPE_ATOM_ID: %s\n", node->svalue);
            break;
        case AST_NODE_TYPE_ATOM_INSTANT:
            node->ivalue = (int)(uint64_t)(node_value);
            AST_LOG("AST_NODE_TYPE_ATOM_INSTANT: %d\n", node->ivalue);
            break;
        case AST_NODE_TYPE_ATOM_OPS_ADD:
        case AST_NODE_TYPE_ATOM_OPS_SUB:
        case AST_NODE_TYPE_ATOM_OPS_MUL:
        case AST_NODE_TYPE_ATOM_OPS_DIV:
        case AST_NODE_TYPE_ATOM_OPS_EQUAL:
        case AST_NODE_TYPE_ATOM_OPS_NOTEQUAL:
        case AST_NODE_TYPE_ATOM_OPS_GREATERTHAN:
        case AST_NODE_TYPE_ATOM_OPS_LESSTHAN:
        case AST_NODE_TYPE_ATOM_OPS_AND:
        case AST_NODE_TYPE_ATOM_OPS_OR:
            AST_LOG("AST_NODE_TYPE_ATOM_OPS(%d)\n", type - AST_NODE_TYPE_ATOM_OPS_ADD);
            break;
        default:
            assert(0);
            break;
    }
    node->lchild = lchild;
    node->rchild = rchild; 
    return node;
}

