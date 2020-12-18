#include <ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <log.h>


#define AST_LOG(...) LOG("line:%d [node:%p lchild:%p rchild:%p] ", yylineno, node, lchild, rchild); LOG(__VA_ARGS__)
#define PARAM_LOG(...) LOG("line:%d [node:%p prev:%p]", yylineno, node, prev); LOG(__VA_ARGS__)
#define STATEMENT_LOG(...) PARAM_LOG(__VA_ARGS__)
#define SCHEMA_LOG(...) PARAM_LOG(__VA_ARGS__)
extern int yylineno;


struct ast_node *
new_ast_node(enum ast_node_type type,
             void * node_value,
             struct ast_node * lchild,
             struct ast_node * rchild,
             ...)
{

    va_list valist;
    va_start(valist, rchild);
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
        case AST_NODE_TYPE_FUNCTION:
            node->functionname = (char *)node_value;
            node->param_list = va_arg(valist, struct param_node *);
            AST_LOG("AST_NODE_TYPE_FUNCTION, function:%s, params:%p\n", node->functionname, node->param_list);
            break;
        default:
            assert(0);
            break;
    }
    node->lchild = lchild;
    node->rchild = rchild;
    
    va_end(valist); 
    return node;
}

struct param_node *
new_param_node(enum param_node_type type,
               struct ast_node * ast,
               struct param_node * prev)
{
    struct param_node * node = (struct param_node *)malloc(sizeof(struct param_node));
    assert(node);
    memset(node, 0x0, sizeof(struct param_node));
    node->node_type = type;
    node->ast = ast;
    node->prev_param = prev;
    if (prev) {
        prev->next_param = node;
    }
    switch(node->node_type)
    {
        case PARAM_NODE_TYPE_VOID:
            PARAM_LOG("PARAM_NODE_TYPE_VOID, head:%p\n", param_list_head(node));
            break;
        case PARAM_NODE_TYPE_AST:
            PARAM_LOG("PARAM_NODE_TYPE_AST:%p head:%p\n", ast, param_list_head(node));
            break;
        default:
            assert(0);
            break;
    }
    return node;
}

struct param_node *
param_list_head(struct param_node * node)
{
    struct param_node * ptr = node;
    for (; ptr->prev_param; ptr = ptr->prev_param);
    return ptr;
}


struct statement_node *
new_statement_node(enum statement_node_type type,
                   struct statement_node * prev,
                   ...)
{
    va_list valist;
    va_start(valist, prev);
    struct statement_node * node = (struct statement_node *)malloc(sizeof(struct statement_node));
    assert(node);
    memset(node, 0x0, sizeof(struct statement_node));
    node->node_type = type;
    node->prev_statement = prev;
    if (prev) {
        prev->next_statement = node;
    }
    switch (node->node_type)
    {
        case STATEMENT_TYPE_VOID:
            STATEMENT_LOG("STATEMENT_TYPE_VOID\n"); 
            break;
        case STATEMENT_TYPE_DECLARE:
            node->variable_name = va_arg(valist, char *);
            node->initializer = va_arg(valist, struct ast_node *);
            STATEMENT_LOG("STATEMENT_TYPE_DECLARE, variable:%s initializer:%p\n",
                          node->variable_name,
                          node->initializer);
            break;
        case STATEMENT_TYPE_ASSIGN:
            node->variable_name = va_arg(valist, char *);
            node->rvalue = va_arg(valist, struct ast_node *);
            STATEMENT_LOG("STATEMENT_TYPE_ASSIGN, variable;%s rvalue:%p\n",
                          node->variable_name,
                          node->rvalue);
            break;
        case STATEMENT_TYPE_RETURN:
            node->return_value = va_arg(valist, struct ast_node *);
            STATEMENT_LOG("STATEMENT_TYPE_RETURN, return value:%p\n", node->return_value);
            break;
        case STATEMENT_TYPE_RAW_EXPRESSION:
            node->raw_expression = va_arg(valist, struct ast_node *);
            STATEMENT_LOG("STATEMENT_TYPE_RAW_EXPRESSION, raw expression:%p\n", node->raw_expression);
            break;
        case STATEMENT_TYPE_CONDITIONAL_EXPRESSION:
            node->conditional_expression = va_arg(valist, struct ast_node *);
            node->hit_statements = va_arg(valist, struct statement_node *);
            node->non_hit_statements = va_arg(valist, struct statement_node *);
            STATEMENT_LOG("STATEMENT_TYPE_CONDITIONAL_EXPRESSION condition:%p, hit statements:%p non-hit statements:%p\n",
                          node->conditional_expression,
                          node->hit_statements,
                          node->non_hit_statements);
            break;
        case STATEMENT_TYPE_LOOP_EXPRESSION:
            node->loop_expression = va_arg(valist, struct ast_node *);
            node->body_statements = va_arg(valist, struct statement_node *);
            STATEMENT_LOG("STATEMENT_TYPE_LOOP_EXPRESSION condition:%p loop body:%p\n",
                          node->loop_expression,
                          node->body_statements);
            break;
        default:
            assert(0);
            break;
    }
    va_end(valist);
    return node;
}

void
concat_statement_nodes(struct statement_node * pnode,
                       struct statement_node * nnode)
{
    assert(pnode && nnode);
    assert(!pnode->next_statement);
    assert(!nnode->prev_statement);
    pnode->next_statement = nnode;
    nnode->prev_statement = pnode;
}

struct statement_node *
statement_list_head(struct statement_node * node)
{
    struct statement_node * ptr = node;
    for (; ptr->prev_statement; ptr = ptr->prev_statement);
    return ptr;
}





struct schema_node *
new_schema_node(enum schema_node_type type,
                char * variable_name,
                struct schema_node * prev)
{
    struct schema_node * node = (struct schema_node *)malloc(sizeof(struct schema_node));
    assert(node);
    memset(node, 0x0, sizeof(struct schema_node));
    node->node_type = type;
    node->variable_name = variable_name;
    node->prev_node = prev;
    if (prev) {
        prev->next_node = node;
    }
    switch(node->node_type)
    {
        case SCHEMA_NODE_TYPE_VOID:
            SCHEMA_LOG("SCHEMA_NODE_TYPE_VOID\n");
            break;
        case SCHEMA_NODE_TYPE_ID:
            SCHEMA_LOG("SCHEMA_NODE_TYPE_ID, variable:%s\n", node->variable_name);
            break;
        default:
            assert(0);
            break;
    }
    return node;
}
