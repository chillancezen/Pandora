#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern int yylex(void);
extern int yyrestart(FILE *);

int
push_and_run(char * target_file);

int
main(int argc, char **argv)
{
    if (argc > 1) {
        int idx = 0;
        for (idx = 1; idx < argc; idx++) {
            //FILE * fp = fopen(argv[idx], "r");
            //assert(fp);
            //yyrestart(fp);
            //while(yylex());
            if (!push_and_run(argv[idx])) {
                while(yylex());
            }
        }
    }
    // Don't read from STDIN
    //yylex();
    return 0;
}
