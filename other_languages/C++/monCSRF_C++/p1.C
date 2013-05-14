#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <iostream>
#include "p1.h"
#include "ParserUtil.h"
#include "DistMetric.h"

#define FIND_OVERLAPS

int printDistMatrix = 0;
#ifdef DBG
int printMatch = 1;
#else
int printMatch = 0;
#endif
int printAllMatches = 0;
int printAllMatchSummary = 0;
int warnNoMatch = 0;
//int printApproxMatches = 1;
int printApproxMatches = 0;

int p1MatchCalls;
double p1MatchLenTot;
double p1MatchLenSqTot;
int p1MatchLenMax;
int p1MatchLenSqMax;
int p1FMatchCalls;
double p1FMatchLenTot;
double p1FMatchLenSqTot;
int p1FMatchLenMax;
int p1FMatchLenSqMax;

 int printPerfStats;

#define MAX_DIST 100000000

#define dist(a, b) dist[(a)*(slen+1)+(b)]

inline void
printDist(int *dist, int slen, int plen) {
  if (printDistMatrix) {
	int i, j;
	printf("  *");
	for (j = 0; j <= slen; j++)
	  printf("%5d", j);
	printf("\n");

	for (i = 0; i <= plen; i++) {
	  printf("%3d", i);
	  for (j = 0; j <= slen; j++)
		printf("%5d", dist(i, j));
	  printf("\n");
	}
  }
}

#define NONE 4
#define SUBST 1
#define DELETE 2
#define INSERT 3

inline void
prtMatch(int b, int e, float d, const char *p, const char *s,
		 const char *op, bool printMatch) {
  int i, j; bool done;
  if (printMatch) {
	printf("s[%d-%d]: %g\n\tp: ", b, e, d);
	for (i=0, j=0, done=false; (!done); j++) {
	  switch (op[j]) {
	  case NONE:
	  case SUBST:
		putchar(p[i]); i++; continue;
	  case DELETE:
		putchar(p[i]); i++; continue;
	  case INSERT:
		putchar('-'); continue;
	  default:
		done=true; break;
	  }
	}
	printf("\n\ts: ");
	for (i=b, j=0, done=false; (!done); j++) {
	  switch (op[j]) {
	  case NONE:
	  case SUBST:
		putchar(s[i]); i++; continue;
	  case INSERT:
		putchar(s[i]); i++; continue;
	  case DELETE:
		putchar('-'); continue;
	  default:
		done=true; break;
	  }
	}
	printf("\n\to: ");
	for (i=b, j=0, done=false; (!done); j++) {
	  switch (op[j]) {
	  case NONE:
		putchar('N'); continue;
	  case SUBST:
		putchar('S'); continue;
	  case INSERT:
		putchar('I'); continue;
	  case DELETE:
		putchar('D'); continue;
	  default:
		done=true; break;
	  }
	}
	printf("\n");
  }
}

inline int getMatch(int *dist, const char *p, const char *s, int plen,int slen,
					int sEnd, DistMetric& dm, float thresh, char *& opmap) {
  // If we need to print the match, then we need to create the
  // string s1 and p1 that are obtained by aligning the matching
  // portion of s with p, and inserting '-' characters where
  // insertions/deletions take place.
  //
  // If we dont need to print, then we need only compute the
  // beginning of the match, which is in "j" at the end of the loop below

  int i = plen, j = sEnd;
  int k = i+sEnd+1;
  int rv;

  char *op = (char *)alloca(k+1);
  op[k--] = 0;

  while ((i > 0) && (j > 0)) {
	char sc = s[j-1], pc = p[i-1];
	int curDist = dist(i, j);
	if (curDist == dist(i-1, j) + dm.delCost(pc)) {
	  op[k] = DELETE;
	  i--;
	}
	else if (curDist == dist(i, j-1) + dm.insCost(sc)) {
	  op[k] = INSERT;
	  j--;
	}
	else {
	  if (dm.subCost(pc, sc) == 0)
		op[k] = NONE;
	  else op[k] = SUBST;
	  i--; j--;
	}
	k--;
  }

  k++;
  rv = j;
  opmap = strdup(&op[k]);
  return rv;
}

MatchRes&
p1Match(const char* p, const int plen, const char *pnm,
		const char* s, const int slen, const char *snm,
		int sOffset, DistMetric& dm, float distThreshold, MatchRes& mres) {
  int i, j, l;
  int sBeg, sEnd;
  int bestDist=MAX_DIST;
  int prevBest;
  int distDn=0, distRt=0, distDiag=0;
  int* dist;
  char *opmap;

  if ((dist = (int *)alloca((plen+1)*(slen+1)*sizeof(int *))) == NULL) {
	printf("dist is NULL!\n");
	exit(-1);
  }

  MatchRes& rv = mres;
  int costThresh;

  if (printPerfStats) {
	p1MatchCalls++;
	p1MatchLenTot += slen;
	p1MatchLenSqTot += slen*plen;
	p1MatchLenMax = max(p1MatchLenMax, slen);
	p1MatchLenSqMax = max(p1MatchLenSqMax, slen*plen);

	if (printAllMatchSummary)
	  if (loglevel >= LOG_INFO)
		cout << "p1Match(p, s[" << sOffset << '-' << sOffset+slen << "])\n";
  }

  // Initialize costThresh from mres parameter. If mres parameter (now in rv)
  // is not initialized, compute it from distThreshold.

  if (rv.costThresh_ < 0) {
	int maxCost = 0;
	prevBest = MAX_DIST;
	for (i = 0; i < plen; i++)
	  maxCost += dm.delCost(p[i]);
	costThresh = (int)floorf(distThreshold * maxCost);
	rv.costThresh_ = costThresh;
  }
  else {
	prevBest = rv.bestDist_;
	costThresh = rv.costThresh_;
  }

  // Initialize distance matrix

  dist(0, 0) = 0;
  for (j=1; j <= slen; j++)
#ifdef EQUALSTRINGS
	dist(0, j) = dist(0, j-1) + dm.insCost(s[j-1]);
#else
	dist(0, j) = 0;
#endif
  for (i=1; i <= plen; i++)
	dist(i, 0) = dist(i-1, 0) + dm.delCost(p[i-1]);
  printDist(dist, slen, plen);

  // Main loop: compute the minimum distance matrix

  for (i = 1; i <= plen; i++) {
	for (j = 1; j <= slen; j++) {
	  char sc = s[j-1], pc = p[i-1];
	  distDn = dist(i-1, j) + dm.delCost(pc);
	  distRt = dist(i, j-1) + dm.insCost(sc);
	  distDiag = dist(i-1, j-1) + dm.subCost(pc, sc);
      dist(i, j) = min3(distDn, distRt, distDiag);
	}
  }
  printDist(dist, slen, plen);

  /* Now, look for j such that dist(plen, j) is minimum.
	 This gives the lowest cost substring match between p and s */

#ifdef EQUALSTRINGS
  if (plen != slen) {
	cout << plen << "!=" << slen << ", exiting\n";
	exit(1);
  }
  sEnd = slen;
  bestDist = dist(plen, slen);
#else
  sEnd = 1;
  for (j=1; j <= slen; j++) {
	if (dist(plen, j) < bestDist) {
	  bestDist = dist(plen, j);
	  sEnd = j;
	}
  }
#endif
  // Compare the best with previous best matches to see if there is any
  // sense in continuing further. We retain results that are below costThresh
  // that are within 50% of costThresh from the best result so far.

  if (bestDist <= prevBest + (costThresh/2)) {
	int bestSoFar = min(bestDist, prevBest);
	rv.bestDist_ = bestSoFar;

	if (bestDist < prevBest) {
	  for (MatchesIt mi=rv.elem_.begin(); mi != rv.elem_.end();) {
		if ((mi->dist_ > costThresh)&&(mi->dist_ > bestDist+(costThresh/2))) {
		  MatchesIt mi1 = mi;
		  mi++;
		  rv.elem_.erase(mi1);
		}
		else mi++;
	  }
	}

    sBeg = getMatch(dist, p, s, plen, slen, sEnd, dm, distThreshold, opmap);
	prtMatch(sBeg, sEnd, bestDist, p, s, opmap, printMatch);

	MatchResElem mre(bestDist, sBeg + sOffset, sEnd-1 + sOffset, opmap);
	rv.elem_.push_front(mre);

	// Now, compare the best match with other possible matches
	// identified in this invocation of this function

#ifdef FIND_OVERLAPS
	for (l=1; l <= slen; l++) {
	  int currDist  = dist(plen, l);
	  if (((currDist <= costThresh) ||
		   (currDist <= bestSoFar + (costThresh/2))) &&
		  // The first two tests below eliminate consideration of distances
		  // that do not correspond to local minima
		  (currDist < dist(plen, l-1)) &&
		  ((l == slen) || currDist < dist(plen, l+1)) &&
		  (l != sEnd)) /* Dont consider the global minima, either */ {

		j = getMatch(dist, p, s, plen, slen, l, dm, distThreshold, opmap);

		/* s[j]...s[l-1] are included in the match with p */

		// Eliminate matches that have large overlap with the main match.
		// This is necessary since the "non-local minima" test earlier misses
		// situations where the distance increases briefly but then
		// decreases after a few more characters. In that case, you seem to
		// have a local minima, but the corresponding match is subsumed in
		// the ultimate match that is discovered.

		int uniqLen=0;
		if (j < sBeg)
		  uniqLen += sBeg-j;
		if (l > sEnd)
		  uniqLen += l-sEnd;
		if (uniqLen > (plen*distThreshold)) {
		  MatchResElem mre(bestDist, j + sOffset, l-1 + sOffset, opmap);
		  rv.elem_.push_front(mre);
		  prtMatch(j, l, dist(plen, l), p, s, opmap, printAllMatches);
		}
	  } // if ((currDist <= costThresh) ...
	} // for (l=1; ...
#endif
  } // if (bestDist <= ...

  return rv;
}

/*
  NOTE: In the fastmatch algorithm below, and its proof of correctness, we
  only consider insertions and deletions, thus treating a substitution as
  a combination of the two. In reality, the D metric treats substitutions
  differently. But the cost function for substitution is set up to be very
  close to insertion+deletion cost except in two cases: case-folding, and
  substitution of one white space character with another. We handle these
  two cases by using the "normalize" function below to map all alphabetic
  characters into upper case and by mapping all white space character to '_'
*/


/*
  The following fastmatch algorithm computes a fast approximation of the
  distance between a string p and substrings of s. It ensures that

  [A] For any s1=s[i...j] such that D(p, s1) <= t, p1FastMatch will
      invoke p1Match(p, s2) for some string s2 that contains s1 as
	  a substring.

  The algorithm operates by comparing p with every substring
  s_k = s[k-plen+1...k] that is of length plen. It computes an
  approximation FD(p, s_k). Let k_1...k_2 be any maximal subrange
  of [0...slen-1] such that FD(p, s_l) <= t' for k_1 <= l <= k_2.
  Then it invokes p1Match(p, s[k_1-plen+1...k_2]).

  To be useful, FD for all s_k should be computable in time that is linear
  in slen. To do this, we define FD as follows. Let P denote the multiset
  of characters in p, i.e., the number of occurrences of each character in
  p is preserved in P, but their order of occurrence isn't. Similarly, let
  S_k denote the multiset of characters in s_k. We define

    P^u_k = P - S_k   [Denotes characters in p that are "unmatched" by
                      characters in S_k. "-" denotes set difference operation.]
	S^u_k = S_k - P   [Chars in S_k unmatched by characters in p]
	FD(p, s_k) = DelCost(P^u_k) + InsCost(S^u_k)

  Note that FD(p, s_k) <= D(p, s_k): in order to make s_k the same as p,
  you will at least need to delete every characters in P^u_k from p, and then
  insert all the characters in S^u_k. However, it is not enough to set a
  the same threshold on FD as D, or otherwise we can't ensure [A]. So we
  define FD in a slightly different way that lets us use a threshold as t:

        FD(p, s_k) = min(DCost(P^u_k), ICost(S^u_k))

  We can compute both the above quantities in an incremental way as k
  goes from plen-1 to slen.

  We will now show that [A] holds if FD is defined as above. There are two
  cases to consider for our proof, depending on whether s1=s[i...j] is longer
  than or shorter than p.

  1. |s1| >= plen. We will show that for every substring s_k of s1,
     ICost(S^u_k) <= t. (Since s1 = s[i...j], it must be the case that
	 i+plen-1 <= k <= j.) Suppose it is not. Note that S^u_k contains the
	 "excess" characters in s_k that aren't matched by any character in p.
	 Since s_k is a substring of s1, all these characters are included in
	 s1 as well, and must be deleted before s1 can be identical to p.
	 Thus D(p, s1) >= ICost(S^u_k) > t, which contradicts the assumption
	 that D(p, s1) <= t.

  2. |s1| < plen. We will show that for the superstring s_j = s[j-plen+1...j]
     of s1, DCost(P^u_j) <= t. Suppose it is not. Then p has P^u_j unmatched
	 characters in s_j. Since s1 is a substring of s_j, all these will
	 continue to be unmatched in s1, which means that
	 D(p, s1) >= DCost(P^u_j) > t,  again contradicting our assumption.

  To show that the definition FD above captures a necessary condition,
  we can give examples in case 1 where D(p, s1) = ICost(S^u_k). Consider
	    xxxxyyyyxxxx     <--- p     P^u_k = {xx}
       xzxxxyyyyxxxzx    <--- s1    S^u_k = {zz}
  Assume ICost and DCost are equal, and are also equal for all characters.
*/

inline void
adjustDiffIns(char c, short vec[], DistMetric& dm, int& idiff, int& ddiff) {
  if (vec[c] > 0) // an excess of c in p, now reduced when c is added to s
	ddiff -= dm.delCost(c);
  else // an excess of c in s, now excess further increased
	idiff += dm.insCost(c);
  vec[c]--;
}

inline void
adjustDiffDel(char c, short vec[], DistMetric& dm, int& idiff, int& ddiff) {
  if (vec[c] >= 0) // an excess of c in p, further increased now
	ddiff += dm.delCost(c);
  else // an excess of c in s, excess being reduced now, so decrease
	idiff -= dm.insCost(c); // insertion cost correspondingly.
  vec[c]++;
}

MatchRes
p1FastMatch(const char* p, int plen, const char *pnm,
			const char* s, int slen, const char *snm,
			float distThreshold, DistMetric& dm) {

  int idiff=0, ddiff=0, diff;
  int diffThresh;
  int start=-1, end=-1;
  short diffVec[256];
  MatchRes rv;

  p1FMatchCalls++;
  p1FMatchLenTot += slen;
  p1FMatchLenSqTot += slen*plen;
  p1FMatchLenMax = max(p1FMatchLenMax, slen);
  p1FMatchLenSqMax = max(p1FMatchLenSqMax, slen*plen);

  rv.costThresh_ = -1;
  rv.bestDist_ = MAX_DIST;

  memset(diffVec,0, sizeof(diffVec));

  for (int i=0; i < plen; i++) {
	char c = dm.normalize(p[i]);
	adjustDiffDel(c, diffVec, dm, idiff, ddiff);
  }

  diffThresh = (int)floorf(ddiff*distThreshold);

  for (int i=0; i < plen; i++) {
	char d = dm.normalize(s[i]);
	adjustDiffIns(d, diffVec, dm, idiff, ddiff);
  }

  diff = min(idiff, ddiff);
  //  assert(idiff==ddiff);
#ifdef VERBOSE
  cout << "diff=" << diff << ", diffThresh=" << diffThresh << endl;
#endif

  if (diff <= diffThresh)
	start = 0;

  for (int j=plen; j < slen; j++) {
	char c = dm.normalize(s[j-plen]);
	char d = dm.normalize(s[j]);

	adjustDiffDel(c, diffVec, dm, idiff, ddiff);
	adjustDiffIns(d, diffVec, dm, idiff, ddiff);
	diff = min(idiff, ddiff);

	if (start == -1) {
	  if (diff <= diffThresh)
		start = j-plen+1;
	}
	else if (diff > diffThresh) {
	  end = j+1;//-1+diffThresh;
	  if (end >= slen)
		end = slen;
	  start -= 1;//diffThresh;
	  if (start < 0)
		start = 0;

#ifdef VERBOSE
	  cout << "Calling p1Match " << start << ", " << end << endl;
#endif
	  p1Match(p, plen, pnm, &s[start], end-start+1, snm, start,
			  dm, distThreshold, rv);
	  start = -1;
	  //j = end-1;
	}
  }

  if (start != -1) {
	end = slen-1;
#ifdef VERBOSE
	cout << "Calling p1Match " << start << ", " << end << endl;
#endif
	p1Match(p, plen, pnm, &s[start], end-start+1, snm, start,
			dm, distThreshold, rv);
  }

  if (printApproxMatches || printAllMatchSummary) {
	if (!rv.elem_.empty() && (rv.bestDist_ <= rv.costThresh_) &&
		(printAllMatchSummary
		 || ((rv.bestDist_ > 0) || (rv.elem_.size() != 1)))) {
	  if (loglevel >= LOG_INFO) {
		cout << "(" << pnm << " Vs " << snm << "): " << endl;
		cout << "         " << p << endl;
	  }
#ifdef VERBOSE
	  for (MatchesIt mi = rv.elem_.begin(); mi != rv.elem_.end(); mi++) {
		prtMatch(mi->matchBeg_, mi->matchEnd_, mi->dist_, p, s, mi->opmap_, true);
	  }
#endif

	  if (loglevel >= LOG_INFO) {
		for (MatchesIt mi = rv.elem_.begin(); mi != rv.elem_.end(); mi++) {
		  int smin = mi->matchBeg_-6;
		  int smax = mi->matchEnd_+6;
		  if (smin > 0)
			cout << "...";
		  else cout << "   ";
		  for (int i=smin; i <= min(smax, slen-1); i++) {
			if (i >= 0)
			  if ((s[i] == '\n') || (s[i] == '\t'))
				cout << ' ';
			  else cout << s[i];
			else cout << ' ';
		  }
		  if (smax < slen-1)
			cout << "...";
		  cout << '[' << mi->matchBeg_ << ',' << mi->matchEnd_ << "]";
		  if (mi->dist_ != 0)
			cout << ": "
				 << (mi->dist_*distThreshold)/rv.costThresh_;
		  cout << endl;
		}
	  }
	}
  }

  return rv;
}

void printMatchPerfStats(ostream& os) {
  if (loglevel >= LOG_INFO) {
	os << "\n\t\tCalls\tAvgLen\tAvLenSq\tMaxLen\tMaxLenSq\n";
	os << "p1Match\t\t"<< p1MatchCalls << '\t'
	   << round(p1MatchLenTot/(1e-9+p1MatchCalls)) << '\t'
	   << round(p1MatchLenSqTot/(1e-9+p1MatchCalls)) << '\t'
	   << p1MatchLenMax << '\t' << p1MatchLenSqMax << '\n';
	os << "p1FMatch\t"<< p1FMatchCalls << '\t'
	   << round(p1FMatchLenTot/(1e-9+p1FMatchCalls)) << '\t'
	   << round(p1FMatchLenSqTot/(1e-9+p1FMatchCalls)) << '\t'
	   << p1FMatchLenMax << '\t' << p1FMatchLenSqMax << '\n';

	os << "Estimated performance improvement by using fast match: "
	   << round(p1FMatchLenSqTot/(1e-9+max(p1MatchLenSqTot, p1FMatchLenTot)))
	   << "x\n";
	os << "Memory savings due to fast match: "
	   << round(p1FMatchLenSqMax/(p1MatchLenSqMax + 1e-9))
	   << "x\n";
  }
}
