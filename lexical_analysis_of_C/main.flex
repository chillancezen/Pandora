%option noyywrap yylineno
%{

#define LINE(desc) printf("line %d: "desc"\n", yylineno)
#define DISPLAY_YYTEXT() printf("    (%d:%s)\n", (int)strlen(yytext), yytext)
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

{WHITESPACE}+ {
    //LINE("whitespaces");
}
. {
    //printf("unrecognized token at line:%d\n", yylineno);
}
%%
