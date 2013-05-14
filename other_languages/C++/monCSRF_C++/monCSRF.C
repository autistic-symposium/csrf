/********************************************************************
 *         monCSRF 1.0 (based on nlearn, seclab stony brook):
 *
 *         1) Read log from APACHE interposition (DIVA).
 *         2) Parse GET requests that have state change:
 *                 - SQL: UPDATE, INSERT INTO, DELETE
 *         3) Whitelist same queries with same syntactical structure.
 *
 *                                          nov/2011
 *******************************************************************/

#include <assert.h>
#include <iostream>
#include <stdio.h>
#include "ParserUtil.h"
#include "STE.h"
#include "SyntaxTree.h"
#include "TreeAuto.h"
#include "DistMetric.h"
#include "SqlInfo.h"

using namespace std;
#define VALID_DIR(d)	((strcasecmp(d,"IN")==0) || (strcasecmp(d,"OUT")==0))


// -------------------- extern variables ---------------------
extern unsigned html_nchars;
extern SymTabEntry global;
SymTabEntry global("GLOBAL", GLOBAL_KIND);
extern void RMSInit();
extern void processMessage(char *dir, char *nm, int nmlen, char *val, int vlen, ostream& sCheck);

// ------------------- global variables ----------------------
TextMetric textm;
DistMetric *dm = &textm;
int loglevel = LOG_INFO;



// **********************************************************
// --------------------- Print Usage ------------------------
// **********************************************************
void prtUsage(const char *prg) {
  cout << "Usage: " << prg << " [-v]"<< " [file1 [file2 ...]]\n";
  cout << "Options: ";
  cout << "-v: verbose mode: prints errors, warnings and debugging info.\n";
}



// **********************************************************
// ------------------- main ---------------------------------
// **********************************************************
//
int main(int argc, const char *argv[]) {

  // ---------- Init Policies -----------------
  RMSInit();

  // ---------- Read arguments ---------------
  for (int nf=1; nf < argc; nf++) {
	FILE *infp=NULL;
	if (argv[nf][0] == '-') {
	  switch(argv[nf][1]) {
	  case 'v':
		loglevel = LOG_DEBUG;
		continue;
	  default:
		prtUsage(argv[0]);
		exit(1);
	  }
	}

        // ------------- Open file ------------
	infp = fopen(argv[nf], FOPEN_READ_MODE);
	if (infp == NULL) {
		perror("Invalid input file name");
		exit(1);
	}

        // ----------- Local Variables ----------
	int k=0, nparms=0;
	char dir[32];
	char fn[256];
	char *nm, *val;
	int nmlen, vlen;
	SymTabEntry *curCtxt=NULL;

        // -------------- Reading Log -----------
        while (fscanf(infp,"%31s%63s%d",dir,fn,&nparms)==3) {
          if (strcmp(fn,"mysql_query")==0){
            printf(fn);
            printf("\n");
          }
	  assert(VALID_DIR(dir));
	  //if(loglevel==LOG_DEBUG)cerr << "Read input " << dir << " " << fn << " " << nparms << endl;
	  if ((nparms < 1) || (nparms > 2)) {
		cout << "Unexpected number of parameters in log entry";
		exit(1);
	  }
	  if (nparms == 1) {
		fscanf(infp, "%d", &vlen);
		nmlen = strlen(fn);
		nm = (char *)malloc(nmlen+1);
		strcpy(nm, fn);
	  }
	  else if (nparms == 2) {
		unsigned l;
		fscanf(infp, "%d%d", &nmlen, &vlen);
		getc(infp); // skip a blank
		l = strlen(fn);
		nm = (char *)malloc(l+nmlen+1);
		strcpy(nm, fn);
		fread(nm+l, nmlen, 1, infp);
		nm[l+nmlen] = '\0';
		if (strlen(nm) < l+nmlen) {
		  cout << "Embedded nulls in log entry %d, skipping";
		  continue;
		}
		else nmlen = strlen(nm);
	  }

          getc(infp); // skip a blank
	  val = (char *)malloc(vlen+3); // extra bytes for flex ...
	  fread(val, vlen, 1, infp);
	  val[vlen] = '\0';

	  // ----------------- Process Message (RmProcess.C)-----
          processMessage(dir, nm, nmlen, val, vlen,  cout);
        }
  }
}

