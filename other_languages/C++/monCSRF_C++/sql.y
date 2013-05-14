%{
#include <iostream>
#include <cstring>
#include "ParserUtil.h"
#include "SyntaxTree.h"

#define paren_conf_set \
   conf = 1.0; \
   if (paren_nesting == 1) \
	 conf *= 0.9; \
   else if (paren_nesting > 1) \
	conf *= 0.6;

#define mulf(f) ((f*conf) > 1.0 ? 1.0 : (f*conf))
#define uc(f) ((unsigned char)(int)(f * 255))

static int cf;
static float conf;
%}

%name-prefix="sql"
%start cmdlist

%union {
  SyntaxTree* stval;
  vector<SyntaxTree*> *vecval;
};

%type <stval>  compoundcmd wordVarComp stmtsep cmdwithops
%type <stval>  op word lparen rparen
%type <vecval> cmdlist cmdlistWithTerms oprparen

%token TOK_STMT_SEP
%token TOK_OPERATOR
%token TOK_OPEN_PAREN
%token TOK_CLOSE_PAREN
%token TOK_WORD
%token TOK_KEYWORD
%token TOK_NUM
%token TOK_COMMENT
%token TOK_OTHER
%token TOK_WS

%%

stmtsep: TOK_STMT_SEP {
  paren_conf_set;
  $$ = new SyntaxTree(STMT_SEP, yylval_beg, yylval_len, 0, uc(conf));
}

op: TOK_OPERATOR {
  paren_conf_set;
  $$ = new SyntaxTree(OPERATOR, yylval_beg, yylval_len, 0, uc(mulf(1.3)));
}

oprparen: op rparen {
  paren_conf_set;
  $$ = new vector<SyntaxTree*>();
  $$->push_back($1);
  $$->push_back($2);
}

word: TOK_WORD {
  paren_conf_set;
  if (strcspn(yylval_beg, "$@") < yylval_len)
	conf *= 0.5;
  else if (strcspn(yylval_beg, "'`\"") < yylval_len)
	conf *= 0.9;
  $$ = new SyntaxTree(CMD_PARAM, yylval_beg, yylval_len, 0, uc(conf));
}
| TOK_KEYWORD {
  paren_conf_set;
  if (strcspn(yylval_beg, "$@") < yylval_len)
	conf *= 0.5;
  else if (strcspn(yylval_beg, "'`\"") < yylval_len)
	conf *= 0.9;
  $$ = new SyntaxTree(CMD_NAME, yylval_beg, yylval_len, 0, uc(conf));
}
| TOK_NUM {
  paren_conf_set;
  $$ = new SyntaxTree(CMD_PARAM, yylval_beg, yylval_len, 0, uc(conf));
}
| TOK_COMMENT {
  $$ = new SyntaxTree(CMD_PARAM, yylval_beg, yylval_len, 0, 5);
  //cout << "Comment node: `" << yylval_beg << "' " << yylval_len << endl;
}
| TOK_WS {
  $$ = new SyntaxTree(CMD_PARAM, yylval_beg, yylval_len, 0, 5);
};

lparen: TOK_OPEN_PAREN {
  paren_conf_set;
  $$ = new SyntaxTree(GRP_BEGIN, yylval_beg, yylval_len, 0, uc(conf));
}

rparen: TOK_CLOSE_PAREN {
  paren_conf_set;
  $$ = new SyntaxTree(GRP_END, yylval_beg, yylval_len, 0, uc(mulf(0.8)));
}

cmdlist:  cmdlistWithTerms {
  $$ = $1;
  inputSynTree = $$;
}
| cmdlistWithTerms cmdwithops {
  $1->push_back($2);
  $$ = $1;
  inputSynTree = $$;
};

cmdlistWithTerms: /* empty */ {
  $$ = new vector<SyntaxTree*>();
}
| cmdlistWithTerms stmtsep {
  $1->push_back($2);
  $$ = $1;
}
| cmdlistWithTerms cmdwithops stmtsep {
  $1->push_back($2);
  $1->push_back($3);
  $$=$1;
}
| cmdlistWithTerms error stmtsep {

	cout << "handling SQL error\n";
  $1->push_back($3);
  $$=$1;
};

cmdwithops: wordVarComp {
  paren_conf_set;
  cf = uc(conf);
  if ($1->nodeType() == CMD_PARAM) {
	conf *= 1.5;
	if (paren_nesting == 0)
	  $1->nodeType(CMD_NAME);
	cf = (int)($1->confidence()*conf);
	if (cf > 255) cf = 255;
	$1->confidence((unsigned char)(cf));
  }
  $$ = new SyntaxTree(CMD, NULL, 0, (unsigned char)(cf), $1);
}
| op wordVarComp {
  paren_conf_set;
  if ($2->nodeType() == CMD_PARAM) {
	conf *= 1.5;
	$2->nodeType(CMD_NAME);
	cf = (int)($1->confidence()*conf);
	if (cf > 255) cf = 255;
	$2->confidence((unsigned char)(cf));
	/*$1->confidence(uc(0.1));*/
  }
  $$ = new SyntaxTree(CMD, NULL, 0, (unsigned char)(cf), $1, $2);
}
| cmdwithops wordVarComp {
  /*
  int i=$1->numChildren();
  if (i > 2) {
	// If we have two consecutive operators in cmdwithops, and the last one
	// is a "+" or a "-", then these are likely prefix operators. We set
    // the confidence low in this case so as to prevent tainting of these
	// prefix symbols from triggering a spanNodes or straddleTrees policy
	SyntaxTree* st1 = $1->child(i-1);
	SyntaxTree* st2 = $1->child(i-2);
	if ((st1->nodeType() == OPERATOR) && (st2->nodeType() == OPERATOR)) {
	  if (st1->value() != NULL)
		if ((strcmp(st1->value(),"-")==0)||((strcmp(st1->value(),"+")==0)))
		  st1->confidence(uc(0.1));
	}
  }
  */

  $1->addChild($2);
  $$ = $1;
}
| cmdwithops op {
  $1->addChild($2);
  $$ = $1;
};

wordVarComp: word {
  $$ = $1;
}
| compoundcmd {
  $$ = $1;
};

compoundcmd: lparen cmdlist rparen {
  paren_conf_set;
  conf *= 0.7;
  $$ = new SyntaxTree(GRP, NULL, 0, $2, uc(conf));
  $$->insert(0, $1);
  $$->addChild($3);
}
| lparen oprparen {
  paren_conf_set;
  conf *= 0.7;
  $$ = new SyntaxTree(GRP, NULL, 0, $2, uc(conf));
  $$->insert(0, $1);
};

%%
