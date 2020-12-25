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
"(" { return LPARENTHESES; }
")" { return RPARENTHESES; }
"=" { return ASSIGN; }
"{" { return LBRACKET; }
"}" { return RBRACKET; }
"," { return COMMA; }
";" { return SEMICOLON; }
"==" { return EQUAL; }
"!=" { return NOTEQUAL; }
">" { return GREATERTHAN; }
"<" { return LESSTHAN; }
"&&" { return AND; }
"||" { return OR; }

"declare" { return DECLARE; }
"define" { return DEFINE; }
"void" { return VOID; }
"return" { return RETURN; }
"if" { return IF; }
"else" { return ELSE; }
"while" { return WHILE; }
"continue" { return CONTINUE; }
"break" { return BREAK; }

[_a-zA-Z][_a-zA-Z0-9]* {
    yylval.sval = strdup(yytext);
    return IDENTIFER;
}
[1-9][0-9]* {
    // only decimal is recognized
    yylval.ival = atoi(yytext);
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
    yylval.ival = atoi(_yytext + 1);
    free(_yytext);
    return NUMBER;
}
. {

}

"\n" { 
    //return EOL;
}

<<EOF>> {
    yyterminate();
}


%%
