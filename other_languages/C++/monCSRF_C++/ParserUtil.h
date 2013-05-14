#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H
using namespace std;

#include <vector>
#include <assert.h>
class SyntaxTree;

extern char *yylval_beg;
extern unsigned   yylval_len;
extern float   yylval_conf;

extern int sqllex();
void sqlerror(char *s);
extern void* sql_setup_buffer(char *base, unsigned int size);

extern vector<SyntaxTree*> *inputSynTree;

extern short paren_nesting;

#define LOG_INFO 5
#define LOG_DEBUG 7

extern int loglevel;
#define dbg(x) if (loglevel >= LOG_DEBUG) { x; }
#define STEP_INDENT 3
#define min3(a, b, c) ( (a) < (b) ? min (a, c) : min (b, c) )

static const char __x[] = "                                                   "
"                                                                           ";
inline void printIndent(ostream& os, unsigned indent) {
  os.write(__x, min(indent, sizeof(__x)-1));
};

#endif
