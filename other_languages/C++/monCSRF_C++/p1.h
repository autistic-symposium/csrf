#ifndef P1_H
#define P1_H

#include <iostream>
using namespace std;
#include "Compatibility.h"

class DistMetric;

class MatchResElem {
 public:
  int dist_;		// edit distance
  int matchBeg_;	// start and
  int matchEnd_;	//   end of match
  char *opmap_;		// table of edit operator costs

 public:
  MatchResElem(int d, int b, int e, char *opmap) 
	{dist_ = d; matchBeg_ = b; matchEnd_ = e; opmap_ = opmap; };
};

typedef slist<MatchResElem> Matches;
typedef slist<MatchResElem>::iterator MatchesIt;

class MatchRes {
// represents a Match Result as returned by p1FastMach
 public:
  int costThresh_;	// minimum edit distance for matches
  int bestDist_;	// best obseved edit distance in the matches found
  Matches elem_;	// list of matches found
} ;

MatchRes p1FastMatch(const char* p, int plen, const char *pnm,
					 const char* s, int slen, const char *snm,
					 float statisticalThreshold, DistMetric& dm);
void printMatchPerfStats(ostream& os = cout);

#endif
