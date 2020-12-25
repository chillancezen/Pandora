#include <ast.h>
#include <stdio.h>
#include <log.h>
#include <assert.h>
#include <string.h>
#include <symbol.h>

struct function_declaration * function_declaration_head = NULL;
struct statement_node * global_statement_head = NULL;

struct symbol * symbols = NULL;

struct  {
    int function_index;
    int conditional_expression_index;
    int loop_expression_index;
} context_indication = {
    .function_index = 0,
    .conditional_expression_index = 0,
    .loop_expression_index = 0   
};

static void
check_function_semantics(struct function_declaration * function);

static void
check_statements_semantics(struct statement_node * statements_head,
                           int visibility_level);

void
register_function(struct function_declaration * function)
{
    check_function_semantics(function);
    if (!function_declaration_head) {
        function_declaration_head = function;
    } else {
        struct function_declaration * ptr = function_declaration_head;
        for (; ptr->next_function; ptr = ptr->next_function);
        ptr->next_function = function;
    }
    LOG("Registered Function: %s\n", function->function_name);
}


void
register_global_statement(struct statement_node * statement)
{
    check_statements_semantics(statement, 0);
    if (!global_statement_head) {
        global_statement_head = statement;
    } else {
        // FIXME: Do not loop, create another rear pointer
        struct statement_node * ptr = global_statement_head;
        for (; ptr->next_statement; ptr = ptr->next_statement);
        concat_statement_nodes(ptr, statement);
    }
    LOG("Registered Statement:%p\n", statement);
}


static void
check_function_schema_semantics(struct schema_node * list_head)
{
    if (!list_head || ((!list_head->next_node) && (list_head->node_type == SCHEMA_NODE_TYPE_VOID))) {
        // legal empty parameter, go ahead.
        return;
    }

    struct schema_node * ptr = list_head;
    for (; ptr; ptr = ptr->next_node) {
        // NON-ID must not appear in parameter list
        ASSERT(ptr->node_type == SCHEMA_NODE_TYPE_ID,
               "semantic error: line %d, parameter should not be an identifier\n",
               ptr->lineno);
        
        // check whether parameter names are duplicated.
        struct schema_node * ptr_another = list_head;
        for (; ptr_another != ptr; ptr_another = ptr_another->next_node) {
            ASSERT(strcmp(ptr->variable_name, ptr_another->variable_name),
                   "semantic error: line %d, found duplidated parameter name:%s\n",
                   ptr->lineno,
                   ptr->variable_name);
        }
        append_symbol(&symbols, new_symbol(SYMBOL_TYPE_PARAMTER, ptr->variable_name, ptr, 1));
    }
}

static void
uncheck_function_schema_semantics(struct schema_node * list_head)
{
    struct schema_node * ptr = list_head;
    assert(ptr);
    for (; ptr->next_node; ptr = ptr->next_node);
    for (; ptr; ptr = ptr->prev_node) {
        if (ptr->node_type != SCHEMA_NODE_TYPE_ID) {
            continue;
        }
        struct symbol * sym = pop_symbol(&symbols);
        assert(sym && !strcmp(sym->symbol_name, ptr->variable_name));
        free_symbol(sym);
    }
}

static void
validate_expression(struct ast_node * expression);

static void
validation_function_expression(struct ast_node * expression)
{
    assert(expression->node_type == AST_NODE_TYPE_FUNCTION);
    struct symbol * function_symbol = NULL;
    ASSERT(function_symbol = lookup_symbol(&symbols, expression->functionname, SYMBOL_TYPE_FUNCTION),
           "semantic error: line %d, function:%s not defined\n",
           expression->lineno,
           expression->functionname);
    struct function_declaration * function_prototype = function_symbol->blob;
    
    // check number of parameters and parameter expression themselves.
    int nr_required_params = 0;
    if (function_prototype->parameter_schema) {
        struct schema_node * _node = function_prototype->parameter_schema;
        for (; _node; _node=_node->next_node) {
            if (_node->node_type == SCHEMA_NODE_TYPE_ID) {
                nr_required_params += 1;
            }
        }
    }
    int nr_given_params = 0;
    if (expression->param_list) {
        struct param_node * _node = expression->param_list;
        for (; _node; _node=_node->next_param) {
            if (_node->node_type == PARAM_NODE_TYPE_AST) {
                nr_given_params += 1;
            }
        }
    }
    ASSERT(nr_required_params == nr_given_params,
           "semantic error: line %d, %d parameters are required but %d are given\n",
           expression->lineno,
           nr_required_params,
           nr_given_params);
    if (expression->param_list) {
        struct param_node * _node = expression->param_list;
        for (; _node; _node=_node->next_param) {
            if (_node->node_type == PARAM_NODE_TYPE_AST) {
                validate_expression(_node->ast);
            }
        }
    }
}

static void
validate_expression(struct ast_node * expression)
{
    if (!expression) {
        return;
    }

    switch (expression->node_type)
    {
        case AST_NODE_TYPE_ATOM_ID:
            ASSERT(is_symbol_present(&symbols, expression->svalue,
                                   SYMBOL_TYPE_GLOBAL_VARIABLE |
                                   SYMBOL_TYPE_PARAMTER |
                                   SYMBOL_TYPE_LOCAL_VARIABLE),
                   "semantic error: line %d, identifier:%s not defined\n",
                   expression->lineno,
                   expression->svalue);
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
            validate_expression(expression->lchild);
            validate_expression(expression->rchild);          
            break;
        case AST_NODE_TYPE_ATOM_INSTANT:
            break;
        case AST_NODE_TYPE_FUNCTION:
            validation_function_expression(expression);
            break;
        default:
            ASSERT(0, "[%s:%d] Assertion Failed", __func__, __LINE__);
            break;
    }
}
static void
validate_variable_declaration(struct statement_node * statement,
                              char * variable_name,
                              int visibility_level)
{
    // Any declaration statements in a loop/conditional expression and in non-function(global) scope are considered as local.
    ASSERT(!is_symbol_present_at_visibility_level(&symbols,
                                                  variable_name,
                                                  visibility_level ? SYMBOL_TYPE_LOCAL_VARIABLE | SYMBOL_TYPE_PARAMTER : SYMBOL_TYPE_GLOBAL_VARIABLE,
                                                  visibility_level),
           "semantic error: line %d, %s variable name:%s has been defined\n",
           statement->lineno,
           visibility_level ? "local" : "global",
           variable_name);
    append_symbol(&symbols, new_symbol(visibility_level ? SYMBOL_TYPE_LOCAL_VARIABLE : SYMBOL_TYPE_GLOBAL_VARIABLE, variable_name, statement, visibility_level));
}

static void
check_statements_semantics(struct statement_node * statements_head,
                           int visibility_level)
{
    if (!statements_head) {
        // No statements; empty statements
        return;
    }
    struct statement_node * statement = statements_head;
    for (; statement; statement=statement->next_statement) {
        switch(statement->node_type) {
            case STATEMENT_TYPE_VOID:
                // empty statement which ends with a comma is allowed.
                break;
            case STATEMENT_TYPE_DECLARE:
                // validate the initialier expression
                // validate the identifer
                validate_expression(statement->initializer);            
                validate_variable_declaration(statement, statement->variable_name, visibility_level);                                
                break;
            case STATEMENT_TYPE_ASSIGN:
                // validate rvalue expression
                validate_expression(statement->rvalue);
                // loopup symbol:lvalue in three places:
                ASSERT(is_symbol_present(&symbols, statement->variable_name,
                                         SYMBOL_TYPE_GLOBAL_VARIABLE | SYMBOL_TYPE_LOCAL_VARIABLE | SYMBOL_TYPE_PARAMTER),
                       "semantic error: line %d, lvalue:%s is not defined\n",
                       statement->lineno,
                       statement->variable_name);
                break;
            case STATEMENT_TYPE_RAW_EXPRESSION:
                validate_expression(statement->raw_expression);
                break;
            case STATEMENT_TYPE_RETURN:
                ASSERT(context_indication.function_index,
                       "semantic error: line %d, return statement is not in a function.\n",
                       statement->lineno); 
                validate_expression(statement->return_value);
                break;
            case STATEMENT_TYPE_CONDITIONAL_EXPRESSION:
                context_indication.conditional_expression_index++;
                validate_expression(statement->conditional_expression);
                check_statements_semantics(statement->hit_statements, visibility_level + 1);
                check_statements_semantics(statement->non_hit_statements, visibility_level + 1);
                context_indication.conditional_expression_index--;
                break;
            case STATEMENT_TYPE_LOOP_EXPRESSION:
                context_indication.loop_expression_index++;
                validate_expression(statement->loop_expression);
                check_statements_semantics(statement->body_statements, visibility_level + 1);
                context_indication.loop_expression_index--;
                break;
            case STATEMENT_TYPE_CONTINUE:
            case STATEMENT_TYPE_BREAK:
                ASSERT(context_indication.loop_expression_index,
                       "semantic error: line %d, continue/break doesn't appear in a loop satement\n",
                       statement->lineno);
                break; 
            default:
                ASSERT(0, "[%s:%d] Assertion Failed", __func__, __LINE__);
                break;
        }
    }

    // reclaim the symbols.
    if (visibility_level) {
        struct statement_node * ptr = statements_head;
        for (; ptr->next_statement; ptr = ptr->next_statement);
        for (; ptr; ptr = ptr->prev_statement) {
            if (ptr->node_type == STATEMENT_TYPE_DECLARE) {
                struct symbol * sym = pop_symbol(&symbols);
                assert(sym && !strcmp(sym->symbol_name, ptr->variable_name) &&
                       sym->type == SYMBOL_TYPE_LOCAL_VARIABLE &&
                       sym->visibility_level == visibility_level);
                free_symbol(sym);
            }
        }
    }
}

static void
check_function_semantics(struct function_declaration * function)
{
    context_indication.function_index++;
    ASSERT(!is_symbol_present(&symbols, function->function_name, SYMBOL_TYPE_FUNCTION),
           "semantic error: line %d, function:%s has already been defined\n",
           function->lineno,
           function->function_name);
    append_symbol(&symbols, new_symbol(SYMBOL_TYPE_FUNCTION, function->function_name, function, 0));
    check_function_schema_semantics(function->parameter_schema);
    check_statements_semantics(function->body, 1);
    uncheck_function_schema_semantics(function->parameter_schema);
    context_indication.function_index--;
}
