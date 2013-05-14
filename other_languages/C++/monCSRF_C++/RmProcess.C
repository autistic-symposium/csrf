/********************************************************************
 *        Parse the log to find GET requestes that
 *        change the state of the system.                                 
 *******************************************************************/


#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "p1.h"
#include "STE.h"
#include "SyntaxTree.h"
#include "TreeAuto.h"
#include "SqlInfo.h"
#include "TEPT.h"
#include "DistMetric.h"

using namespace std;
#define VALID_DIR(d)	((strcasecmp(d,"IN")==0) || (strcasecmp(d,"OUT")==0))

// ----------------------- Global Variables -------------
SqlInfo *whitelistedSqls[1000];
int whitelistedSqlsCounter = 0;
enum OutputType {UNKNOWN_FORMAT, SQL_STATE_CHANGE};
unsigned yylval_len;
char *yylval_beg;


// ---------------- Extern Variables and Classes ----------
extern SymTabEntry global;
Context *curCtxt=NULL;
extern int loglevel;
extern int sqllineno;
extern char *sqltext;
extern int sqlparse();
extern int sqldebug;
vector<SyntaxTree*> *inputSynTree;
extern DistMetric *dm;
TEPT *permCrossNodeTaintSQL;
TEPT *permStraddleTreesTaintSQL;
RegExpr *SQLstateChangeFilter;



// ------------------- Syntax Tree CALL PARSE ---------
static SyntaxTree* callParse(char *s, void* (*setupFn)(char *s, unsigned n), int (*doParse)()) {
  SyntaxTree *rv = NULL;
  unsigned nc = strlen(s);
  if (doParse == &sqlparse) {
    s[nc++] = '\n';
  }
  s[nc++] = '\0';
  s[nc++] = '\0';
  if ((*setupFn)(s, nc) == NULL)
    cout << "ERROR: Buffer setup failed\n";
  else {
    cout << "Calling parse ``" << s << "''\n";
    (*doParse)();
    if (inputSynTree != NULL) {
      rv = new SyntaxTree(ROOT_NODE, s, 0, inputSynTree);
      dbg(
          cout << "Parsed ``";
          for (int i=0; i < inputSynTree->size(); i++)
          (*inputSynTree)[i]->printall(cout, 2);
          cout << "''\n";
         );
    }
    else cout << "ERROR: parse failed\n";
  }
  return rv;
}


// ------------- Syntax Tree PARSE ---------------------
static SyntaxTree* parse(OutputType op, char *s) {
  SyntaxTree *rv = NULL;
  switch (op) {
    case SQL_STATE_CHANGE:
      rv = callParse(s, &sql_setup_buffer, &sqlparse); break;
    default:
      rv = NULL;
  }
  return rv;
}


// -------------------- PARSE ERRORS -------------------
void shellerror(char *s){
  cout << " line "  <<  ": "<< "syntax error `" << yylval_beg << "'"<< endl;}

void sqlerror(char *s){
  cout << " line " << sqllineno <<  ": "<< "syntax error `" << yylval_beg << "'"<< endl;}



// --------------------- PROCESS IN PARAM ---------------
void processInParam(char *nm, char *str, Context* ctxt, ostream& sOutput) {
  assert(ctxt != NULL);
  if (strcasecmp(nm, "postbody") == 0) {
    SyntaxTree *st = NULL;
  }
  else {
    SymTabEntry *ste = ctxt->insertNewIn(nm);
    assert(ste->kind() == INPUT_PARAM_KIND);
    ste->incAccCount();
    if (str != NULL)
      ctxt->cursession()->insertIn(*ste, str);
    else cout << "insertIn: str is NULL for " <<  ste << endl;
  }
}



// ------------------ ADD SQL TO A WHITELIST ----------
void addSqlToWhiteList(SqlInfo *sql_info) {
  ofstream whitelist_file;
  whitelist_file.open("whitelist.txt", ios::app);
  whitelist_file << sql_info->getSqlCommand() << "\n";
  whitelist_file.close();
  whitelistedSqls[whitelistedSqlsCounter] = sql_info;
  whitelistedSqlsCounter++;
}



// ------------ CHECK IF WHITELISTED -----------------
bool whitelistedSql(string output_sql) {
  SqlInfo *output_sql_info;
  output_sql_info = new SqlInfo(output_sql);//where we parsed
  string response;
  for(int i = 0; i < whitelistedSqlsCounter; i++) {
    if (output_sql_info->compare(whitelistedSqls[i])) {
      return true; // Whitelist match
    }
  }
  cout << "The SQL request: <" << output_sql_info->getSqlCommand() << "> is not on the whitelist. Do you want to add?(y/n)[n]\n";
  cin  >> response;
  if (response.compare("y") == 0){
    addSqlToWhiteList(output_sql_info);
    if(loglevel==LOG_DEBUG){
      cout << "SQL query whitelisted.\n";
    }
    return true;
  }
  return false; // No whitelist match
}



// ---------- LOAD WHITELIST AND WRITE INTO IT ---------
void parseWhitelistFile() {
  ifstream whitelist_file;
  string tmp_query;
  whitelist_file.open("whitelist.txt");
  if (whitelist_file){
    while (getline(whitelist_file, tmp_query)) {
      whitelistedSqls[whitelistedSqlsCounter] = new SqlInfo(tmp_query);
      whitelistedSqlsCounter++;
    }
    whitelist_file.close();

  } else {
    cout << "ERROR: whitelist.txt could not be opened";
  }
}



// --------------- PROCESS STATE CHANGE ---------------
 void processStateChange(const char* in_verb, const char* in_path, const char *out_type, const char *out_value) {
  if (strcmp(in_verb, "GET") == 0) {
    if (whitelistedSql(out_value)) { // Checks on the whitelist for the query
      return; // Whitelisted, get out of here.
    }
  }
}



// ------------------ PROCESS OUT PARAM ----------------
void processOutParam(char *nm, char *str, Context* ctxt, ostream& sOutput) {
  assert(ctxt != NULL);
  OutParam* op = ctxt->insertNewOut(nm);
  op->incAccCount();
  OutputType optype = UNKNOWN_FORMAT;
  if ((strstr(nm, "sql") != NULL) && ((strstr(str, "UPDATE") != NULL) || (strstr(str, "INSERT INTO") != NULL) || (strstr(str, "DELETE") != NULL)))
    optype = SQL_STATE_CHANGE;
  TEPT *crossNodesPolicy=NULL, *straddleTreesPolicy=NULL, *policy=NULL;
  RegExpr *inputFilter=NULL;
  const char *atkmsg = NULL;
  switch (optype) {
    case SQL_STATE_CHANGE:
      crossNodesPolicy = permCrossNodeTaintSQL;
      straddleTreesPolicy = permStraddleTreesTaintSQL;
      policy = &problemUpdate;
      policy = &noTaintedCmdPolicy;
      inputFilter = SQLstateChangeFilter;
      atkmsg = "SQL_STATE_CHANGE";
      break;
    default:
      return;
  }
  SyntaxTree *st = NULL;
  if (optype != UNKNOWN_FORMAT) {
      unsigned m = ctxt->cursession()->ninputs();
      for (int i=0; i < m; i++) {
        const char* pnm = ctxt->inputNm(i);
        const char* p = ctxt->cursession()->inputval(i);
        if ((pnm == NULL) || (p == NULL)) continue;
        crossNodesPolicy->clearMatched();
        straddleTreesPolicy->clearMatched();
        if (optype == SQL_STATE_CHANGE) {
          processStateChange(pnm, p, nm, str);
        }
      }
  }
}



// ------------------ SETUP NEW CONTEXT -------------------
bool setupNewContext(char *nm, int nmlen, char *val, int vlen) {
  char *p = (char *)strstr(val, "?");
  if (p != NULL)
    *p = '\0';
  char * ctxtName = (char *)alloca(vlen+nmlen+2);
  if (!ctxtName) {
    printf("failed to allocate ctxtName\n");
    return false;
  }
  strcpy(ctxtName, nm);
  ctxtName[nmlen] = ' ';
  memcpy(ctxtName+nmlen+1, val, vlen);
  ctxtName[vlen+nmlen+1] = '\0';
  SymTabEntry *stp;
  if ((stp = global.lookUp(ctxtName)) == NULL) {
    curCtxt = new Context(ctxtName);
    global.insert(curCtxt);
  }
  else {
    assert(stp->kind() == CONTEXT_KIND);
    curCtxt = (Context*) stp;
  }
  assert(curCtxt != NULL);
  curCtxt->incAccCount();
  curCtxt->newsession();
  return true;
}



// ------------- PROCESS MESSAGE ------------------
void processMessage(char *dir, char *nm, int nmlen,
    char *val, int vlen,  ostream& sCheck) {
  if (strcasecmp(dir, "IN") == 0) {
    if ((strcmp(nm, "GET") == 0) || (strcmp(nm, "PUT") == 0)
        || (strcmp(nm, "POST") == 0))
      if (!setupNewContext(nm, nmlen, val, vlen))
        return;
    processInParam(nm, val, curCtxt, sCheck);
  }
  else processOutParam(nm, val, curCtxt, sCheck);
}



// --------------- INIT CROSS NODE TAINT (TEPT.C)-------------
TEPT * initCrossNodeTaint(NodeTypeSet rootPos, NodeTypeSet leftPos, NodeTypeSet rightPos, const char *left=NULL) {
  TEPT* permLeftChild = new TEPT(leftPos, 0.0, left);
  TEPT *permRightChild = new TEPT(rightPos, 0.0);
  permLeftChild->pos(1);
  permRightChild->pos(2);
  TEPT* rv = new TEPT(rootPos,0.0,NULL,0,permLeftChild,permRightChild);
  return rv;
}

TEPT * initCrossNodeTaint() {
  return initCrossNodeTaint(NodeTypeSet(), NodeTypeSet(), NodeTypeSet());
};


// -------------- INIT POLICES -------------------------
void RMSInit() {
  permCrossNodeTaintSQL =  initCrossNodeTaint(NodeTypeSet(CMD), NodeTypeSet(OPERATOR), NodeTypeSet(CMD_PARAM), "[-+]");
  permStraddleTreesTaintSQL = initCrossNodeTaint();
  SQLstateChangeFilter = new RegExpr("UPDATE|INSERT INTO|DELETE|update|delete|create");
  pcrecpp::RE_Options opt;
  opt.set_caseless(true);
  parseWhitelistFile();
}
