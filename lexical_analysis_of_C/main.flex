%option noyywrap yylineno
%{
#include <assert.h>

struct processing_frame {
    struct processing_frame * previous;
    YY_BUFFER_STATE bs;
    int lineno;
    char * filename;
    FILE * file;
};

#define LINE(desc) printf("%s:%d: "desc"\n",current_file, yylineno)
#define DISPLAY_YYTEXT() printf("    (%d:%s)\n", (int)strlen(yytext), yytext)

struct processing_frame * stack_top = NULL;
char * current_file = NULL;
struct processing_frame * new_pframe(void);
int pop_and_run(void);
int push_and_run(char *);

%}

WHITESPACE [ \t\n]

TYPE int|char|void|short|float

KEYWORD0 auto|break|case|const|contiue|default|do|else|enum
KEYWORD1 extern|for|goto|if|inline|register|return|signed|sizeof
KEYWORD2 static|struct|switch|typedef|union|unsigned|volatile|while
KEYWORD {KEYWORD0}|{KEYWORD1}|{KEYWORD2}

OPERATOR0 "+"|"-"|"*"|"/"|"%"|"++"|"--"
OPERATOR1 "=="|"!="|">"|"<"|">="|"<="|"&&"|"||"|"!"
OPERATOR2 "&"|"|"|"^"|"<<"|">>"
OPERATOR3 "="|"+="|"-="|"*="|"/="|"%="
OPERATOR4 "?"|":"
OPERATOR5 "&"|"*"
OPERATOR {OPERATOR0}|{OPERATOR1}|{OPERATOR2}|{OPERATOR3}|{OPERATOR4}|{OPERATOR5}

PUNCTUATORS "{"|"}"|"["|"]"|";"

%%

\"([^\"\n]|(\\.))*\" {
    LINE("string");
    DISPLAY_YYTEXT();
}

{TYPE} {
    LINE("defined:type");
    DISPLAY_YYTEXT();
}

{KEYWORD} {
    LINE("defined:keyword");
    DISPLAY_YYTEXT();
}

{OPERATOR} {
    LINE("defined:operator");
    DISPLAY_YYTEXT();
}

{PUNCTUATORS} {
    LINE("defined:punctuator");
    DISPLAY_YYTEXT();
}

('[^\\]')|('\\[abfnrtv'"?\\]')|('\\0[0-7]{0,2}')|('\\x[0-9a-zA-Z]{1,2}') {
    LINE("character");
    DISPLAY_YYTEXT();
}

[-+]?((0[xX][0-9a-fA-F]+)|([0-9]+)) {
    LINE("integer");
    DISPLAY_YYTEXT();
}
[_a-zA-Z][_a-zA-Z0-9]* {
    LINE("identifier");
    DISPLAY_YYTEXT();
}
"//".* {
    LINE("single line comment");
}
"/*" {
    // There is no good way to match a multi-line comment, so we resolve it mannually
    char ch;
    char last_ch = '\x0';
    while((ch = input())) {
        if (last_ch == '*' &&
            ch == '/') {
            break;
        }
        last_ch = ch;
    }
    LINE("multiple lines comment");
}
[ \t]*(#)[ \t]*(include)[ \t]*[<"] {
    char left_open_character = 0;
    char right_closing_character = '"';
    {
        char * _ptr = yytext;
        for(; *_ptr; _ptr++) {
            left_open_character = *_ptr;
        }
        if (left_open_character == '<') {
            right_closing_character = '>';
        }
    }
    char path[256];
    int ipath = 0;

    {
        char _ch = 0;
        while ((_ch = input()) &&
               (_ch != right_closing_character) &&
               (_ch != '\n') &&
               ipath < 255) {
            path[ipath++] = _ch;
        }
        if (_ch == '\n') {
            yylineno++;
        }
        path[ipath] = '\x0';
    }
    printf("scanning included file:%s:%d\n", path, yylineno);
    push_and_run(path); 
}

{WHITESPACE}+ {
    //LINE("whitespaces");
}
. {
    //printf("unrecognized token at line:%d\n", yylineno);
}
<<EOF>> {
    if (!pop_and_run()) {
        return 1;
    }
    return 0;
}
%%


struct processing_frame *
new_pframe(void)
{
    struct processing_frame * frame = (struct processing_frame *)malloc(sizeof(struct processing_frame));
    assert(frame);
    memset(frame, 0x0, sizeof(struct processing_frame));
    return frame;
}


int
push_and_run(char * target_file)
{
    // Check whether target file is already included to avoid nested inclusion
    {
        struct processing_frame * ptr = stack_top;
        while (ptr) {
            if (!strcmp(target_file, ptr->filename)) {
                fprintf(stderr, "nested inclusion found for file:%s\n", target_file);
                {
                    struct processing_frame * _ptr = stack_top;
                    printf("inclusion path stack:\n");
                    while (_ptr) {
                        printf("\t:%s\n", _ptr->filename);
                        _ptr = _ptr->previous;
                    }
                }
                exit(-1);
            }
            ptr = ptr->previous;
        }
    }

    FILE * fp = fopen(target_file, "r");
    if (!fp) {
        fprintf(stderr, "unable to open %s!\n", target_file);
        return -1;
    }
    struct processing_frame * pframe = new_pframe();
    //pframe->previous = stack_top;
    pframe->bs = yy_create_buffer(fp, YY_BUF_SIZE);
    if (stack_top) {
        stack_top->lineno=yylineno;
    }
    pframe->filename = strdup(target_file);
    pframe->previous = stack_top;
    pframe->file = fp;
    stack_top = pframe;
    current_file = target_file;
    yylineno = 1;
    yy_switch_to_buffer(pframe->bs);
    return 0;
}


int
pop_and_run(void)
{
    assert(stack_top);
    struct processing_frame * pframe = stack_top;
    stack_top = pframe->previous;

    free(pframe->filename);
    fclose(pframe->file);
    yy_delete_buffer(pframe->bs);
    free(pframe);

    if (!stack_top) {
        return -1;
    }
    yy_switch_to_buffer(stack_top->bs);
    current_file = stack_top->filename;
    yylineno = stack_top->lineno;
    return 0; 
}
