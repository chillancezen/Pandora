%option noyywrap yylineno


%{
    #include <calc.token.h>

    //extern int yylval;
%}

%%

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"(" { return LPARATHESES; }
")" { return RPARATHESES; }

[1-9][0-9]* {
    // only decimal is recognized
    yylval = atoi(yytext);
    return NUMBER;
}

"|"[ \t]*[-+]?[ \t]*[1-9][0-9]*[ \t]*"|" {
    char * _yytext = strdup(yytext);
    char * _ptr = _yytext + 1;
    for (; *_ptr; _ptr++) {
        if (*_ptr == '|') {
            *_ptr = '\x0';
            break;
        }
    }
    yylval = atoi(_yytext + 1);
    free(_yytext);
    return NUMBER;
}
. {

}

"\n" { return EOL; }

<<EOF>> {
    return EOL;
}


%%
