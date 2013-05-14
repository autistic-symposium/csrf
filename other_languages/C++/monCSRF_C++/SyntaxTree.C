#include "SyntaxTree.h"
#include "TreeAuto.h"
#include "TEPT.h"
#include <math.h>
#include "DistMetric.h"

extern TextMetric textm;
const float statisticalThreshold=0;
const int wordTaintThreshLen=0;
const int taintThreshLen=0;

extern int nShared, nTotal, nTaintSharProb;
bool eqSTree(const SyntaxTree* st1, const SyntaxTree* st2) {
  return st1->shallowEqTreeVal(st2);
};
bool lessSTree(const SyntaxTree* st1, const SyntaxTree* st2) {
  return st1->shallowLessTreeVal(st2);
}
size_t hashSTree(const SyntaxTree* st) { return st->shallowHashTreeVal(); };

int min(int a, u_short b) {
  return a < b ? a : b;
}


inline void SyntaxTree::
initFields(NodeType nt, char *beg, int len, u_char s,
		   vector<SyntaxTree*> *child) {
  nodeType(nt);
  isParameterized(false);
  isSubTreeTainted(false);
  val_ = beg;
  vlen_ = len;
  confidence_ = s;
  taintInfo_ = NULL;
  child_ = child;

  assert((val_ != NULL) || (vlen_ == 0));
}

SyntaxTree::
SyntaxTree(NodeType nodeType, char *beg, int len, u_char s) {
  initFields(nodeType, beg, len, s);
}

SyntaxTree::
SyntaxTree(NodeType nodeType, char *beg, int len,
		   vector<SyntaxTree*> *child, u_char s) {
  initFields(nodeType, beg, len, s, child);
}

SyntaxTree::
SyntaxTree(NodeType nodeType, char *beg, int len, u_char s, SyntaxTree *child1,
			 SyntaxTree* child2) {
	initFields(nodeType, beg, len, s);
  if (child1 != NULL) addChild(child1);
  if (child2 != NULL) addChild(child2);
}

SyntaxTree::
SyntaxTree(NodeType nodeType, SyntaxTree *child1, SyntaxTree* child2) {
  initFields(nodeType, NULL, 0, 255);
  if (child1 != NULL) addChild(child1);
  if (child2 != NULL) addChild(child2);
}

int SyntaxTree::
cmpNodeVal(const SyntaxTree* st) const {
  if (this == NULL)
	if (st == NULL)
	  return 0;
    else return -1;
  else if (st == NULL)
	return 1;
  else if (flags_ == st->flags_) {
	if (vlen_ == st->vlen_)
	  return memcmp(val_, st->val_, vlen_);
	else if (vlen_ < st->vlen_)
	  return -1;
	else return 1;
  }
  else if (flags_ < st->flags_)
	  return -1;
  else return 1;
}

inline static int
mystrcmp(const char *s1, const char *s2) {
  if (s1 == s2)
	return 0;
  else if ((s1 == NULL) && (s2 != NULL))
	return -1;
  else if ((s1 != NULL) && (s2 == NULL))
	return 1;
  else return strcmp(s1, s2);
}

int SyntaxTree::
shallowCmpTreeVal(const SyntaxTree* st) const {
  int rv = cmpNodeVal(st);
  if ((rv == 0) && (this != NULL)) {
	int nc1 = numChildren();
	int nc2 = st->numChildren();
	if (nc1 < nc2)
	  return -1;
	else if (nc2 < nc1)
	  return 1;
	else {
	  for (int i=0; (i < nc1); i++) {
		if (child(i) == st->child(i))
		  continue;
		else if (child(i) < st->child(i))
		  return -1;
		else return 1;
	  }
	  if ((!isParameterized()) && (!st->isParameterized())) {
		bool vt1 = isNodeTainted();
		bool vt2 = st->isNodeTainted();
		if (!vt1 && !vt2)
		  rv= 0;
		else if (!vt1 && vt2) {
		  rv = -1;
		}
		else if (vt1 && !vt2) {
		  rv = 1;
		}
		else {
		  if (taintInfo_->len_ < st->taintInfo_->len_)
			rv = -1;
		  else if (taintInfo_->len_ > st->taintInfo_->len_)
			rv =  1;
		  else if ((taintInfo_->begin_-val_)<(st->taintInfo_->begin_-st->val_))
			rv = -1;
		  else if ((taintInfo_->begin_-val_)>(st->taintInfo_->begin_-st->val_))
			rv = 1;
		  else rv = 0;
		}
		if (rv != 0) dbg(nTaintSharProb++);
		return rv;
	  }
	  else if ((!isParameterized()) && (st->isParameterized()))
		  rv = -1;
	  else if ((isParameterized()) && (!st->isParameterized()))
		  rv = 1;
	  else {
		if (paramInfo_ == st->paramInfo_)
		  return 0;
		else if ((paramInfo_ == NULL) && (st->paramInfo_ != NULL))
		  rv = -1;
		else if ((paramInfo_ != NULL) && (st->paramInfo_ == NULL))
		  rv = 1;
		else { // neither is NULL
		  rv = mystrcmp(paramInfo_->paramName_, st->paramInfo_->paramName_);
		  if (rv == 0)
			rv = mystrcmp(paramInfo_->constPrefix_,
						  st->paramInfo_->constPrefix_);
		  if (rv == 0)
			rv = mystrcmp(paramInfo_->constSuffix_,
						  st->paramInfo_->constSuffix_);
		  if (rv == 0)
			rv = mystrcmp(paramInfo_->inputEdits_,
						  st->paramInfo_->inputEdits_);
		}
	  }
	}
  }
  return rv;
}

size_t SyntaxTree::
hashNodeVal() const {
  return hashString(val_, vlen_)*(flags_+1);
}

size_t SyntaxTree::
shallowHashTreeVal() const {
  int nc = numChildren();
  size_t rv = hashNodeVal();
  for (int i=0; (i < nc); i++)
	rv = (rv ^ (unsigned)child(i));
  return rv;
}

inline void
mywrite(ostream& os, const char *s, int len, int indent) {
  for (int i=0; (i < len) && (*s != '\0'); i++, s++) {
	os << *s;
	if (*s == '\n')
	  printIndent(os, indent);
  }
}

const char*
getNodeName(NodeType nt) {
  switch (nt) {
  case ROOT_NODE:  return ("ROOT"); break;
  case STMT_SEP: return ("\n"); break;
  case OPERATOR: return ("+"); break;
  case CMD: return ("CMD"); break;
  case CMD_NAME: return ("N"); break;
  case CMD_PARAM: return ("P" ); break;
  case CMD_REFFER: return ("R" ); break;
  case PARAM_NAME: return ("PN="); break;
  case PARAM_VAL: return ("PV="); break;
  case GRP: return ("GRP"); break;
  case GRP_BEGIN: return ("<<<"); break;
  case GRP_END: return (">>>"); break;
  case BODY: return ("_"); break;
  case INPUT_NODE: return("I");break;
  case OUTPUT_NODE: return("O");break;
  case MULTIPLE: return ("*" ); break;
  default: return ("ERRNODE"); break;
  }
};

void
printEdits(const char *e, ostream& os) {
  if (e != NULL) {
	os << ":"; // etc. TBD
  }
}
void SyntaxTree::
print(ostream& os, int prtNodeFlags, int depthThresh, u_char confThresh,
	  unsigned indent, bool preserveWS) const {
  //confThresh = 0;
  if ((isParameterized()) && (paramInfo_ != NULL)) {
	os << paramInfo_->constPrefix_ << "[@" << paramInfo_->paramName_;
	printEdits(paramInfo_->inputEdits_, os);
	os << "]" << paramInfo_->constSuffix_;
  }
  else if ((val_ != NULL) && (vlen_ > 0) && (confidence_ > confThresh)) {
	if (isNodeTainted()) {
	  mywrite(os, val_, min(taintInfo_->begin_-val_,vlen_),indent);
	  os << "[";
	  mywrite(os, taintInfo_->begin_,
			  min(taintInfo_->len_, vlen_-(taintInfo_->begin_-val_)), indent);
	  os << "]";

	  if (prtNodeFlags & 0x2) {
		os << ": " << taintInfo_->srcNm_ << " ";
	  }

	  mywrite(os,taintInfo_->begin_+taintInfo_->len_,
			  vlen_-taintInfo_->len_+(val_-taintInfo_->begin_),indent);
	}
	else mywrite(os, val_, vlen_, indent);
  }

  const char* nodename = getNodeName(nodeType());

  if ((prtNodeFlags & 0x1) && (nodename != NULL)) {
	if (*nodename != '\n')
	  if (vlen_ > 0)
		os << ':';
	os << nodename;
	if (*nodename == '\n')
	  printIndent(os, indent);
	if (confidence_ < 250)
	  os << (int)confidence_ << ' ';
	if ((numChildren() > 0) && (depthThresh > 0)) {
	  os << "{[";
	  indent += STEP_INDENT;
	  os << '\n';
	  printIndent(os, indent);
	}
  }
  if ((depthThresh > 0) && (nodeType() == GRP) && (numChildren() > 0))
	indent += STEP_INDENT;

  if ((depthThresh > 0) && (numChildren() > 0))
	for (int i=0; i < numChildren(); i++)
	  child(i)->print(os, prtNodeFlags, depthThresh-1, confThresh,
					  indent, preserveWS);

  if ((prtNodeFlags & 0x1) && (depthThresh > 0) && (numChildren() > 0)) {
	indent -= STEP_INDENT;
	os << '\n';
	printIndent(os, indent);
	os << "]}";
	os << '\n';
	printIndent(os, indent);
  }
  if ((vlen_ != 0) && (nodeType() != STMT_SEP) && (!preserveWS))
	os << ' ';
}

bool SyntaxTree::
initTaint(const char *s, const char *pnm, const char *p,
		  int mb, int me, char *edits, TEPT* spanNodesPolicy,
		  TEPT* straddleTreesPolicy, bool *ltm, bool *rtm) {
  int b, e; //=(1<<30), e=0;
  // rmtc(lmtc): right(left) end of node tainted, plus node is mostly tainted
  // These values flow into rtm(ltm) eventually.

  bool lmt=false, lmtc=false;
  bool rmt=false, rmtc=false;
  if (rtm == NULL) rtm = &rmt;
  if (ltm == NULL) ltm = &lmt;

  b = val_ - s;
  e = b+vlen_-1;

  /* Following cases arise in terms of possible ways the string for
	 this node (pointed to by val_) overlaps with the match:
	                            b	                 e
         Case 1   mb    me
		 Case 2                                                 mb     me
		 Case 3.1                            mb            me
		 Case 3.2                            mb me
		 Case 4.1          mb                   me
         Case 4.2          mb                              me
  */

  if ((me >= b) && (mb <= e) /*&& (confidence() > 50)*/) {
	// Rule out untainted cases 1, 2// don't mark taint when parse conf is low
	unsigned taintLen, taintSrcOffset;
	const char* taintBegin;
    if ((me - mb+1) >= taintThreshLen) {
	  dbg(cout<<"mb="<<mb <<" me="<<me<<" b="<< b<<" e="<<e<<endl);
	  rmtc = ((me >= e) && ((e-mb+1 >= wordTaintThreshLen) ||
						  ((e-mb+1)/(e-b+1) > (1-statisticalThreshold))));
	  lmtc = ((mb <= b) && ((me-b+1 >= wordTaintThreshLen) ||
						  (me-b+1)/(e-b+1) > (1-statisticalThreshold)));
	}
	if (b <= mb) { // Note: b <= mb <= e, cases 3.1 and 3.2
	  taintBegin = s + mb;
	  taintSrcOffset = 0;
	  taintLen = min(e, me)-mb+1;
	}
	else { // Note: mb < b <= me, cases 4.1 and 4.2
	  taintBegin = val_;
	  taintSrcOffset = b-mb;
	  taintLen = min(e, me)-b+1;
	}

	if (taintInfo_ == NULL)
	  taintInfo_ = new TaintInfo();
	if (taintLen > taintInfo_->len_) { //@@@@ if >1 source, pick longer taint
	  taintInfo_->len_ = taintLen;
	  taintInfo_->begin_ = taintBegin;
	  taintInfo_->edits_ = edits;
	  taintInfo_->src_ = p;
	  taintInfo_->srcNm_ = pnm;
	  taintInfo_->srcOffset_ = taintSrcOffset;
	  //isTainted_ = isSubTreeTainted_ = true;
	}
  }

  bool ltm1=false, rtm1=rmtc; // ltm1 and rtm1 hold values of lmtc and rmtc
       // after possible updates in calls to initTaint children nodes
  int nc = numChildren();
  if ((nc > 0) && ((nodeType() == OPERATOR)||(b < me))) { //initTaint children
	bool lrtm, lltm; // Previous values of rtm and ltm
	bool c;
	for (int i=0; i < nc; i++) {
	  SyntaxTree &cst = *(*child_)[i];
	  lrtm = rtm1; lltm = ltm1;
	  c = cst.initTaint(s, pnm, p, mb, me, edits,
						spanNodesPolicy, straddleTreesPolicy, &ltm1, &rtm1);
	  isSubTreeTainted(isSubTreeTainted() || c);
	  if (i == 0) {*ltm = ltm1; dbg(cout << "SPAN/STRADDLE: i=0\n");}
	  else if ((i > 0) && lrtm && ltm1) {
		SyntaxTree* res=NULL;
		SyntaxTree* nst = replaceChildren((*child_)[i-1], &cst);
		if (spanNodesPolicy) {
		  // Note that span/straddle policies are "permit" policies.
		  // So, if match succeeds, then there is no problem. Otherwise
		  // we have violation of the permit policy, and hence we
		  // need to "invert" the direction of match. At the same time,
		  // we need to keep track of previous matches
		  SyntaxTree *prevmatch = spanNodesPolicy->matched();
		  res = spanNodesPolicy->nodeEnforce(nst);
		  if (res == NULL) {
			spanNodesPolicy->matched(this, (*child_)[i-1], &cst);
		  }
		  else {
			spanNodesPolicy->matched(prevmatch);
		  }
		}
		dbg(cout << "\nSPAN " << ((res == NULL) ? "(Y)" : "(N)") << " `";
			child(i-1)->printall(cout,1); cout << "' `";
			child(i)->printall(cout,1);
			cout << "':" << getNodeName(nodeType()) << "\n");
		if ((straddleTreesPolicy) && ((!lltm) || (!rtm1))) {
		  // Since other TEPT policies are "deny" policies, we need to
		  // negate the semantics of TEPT match to support the "permit"
		  // policies we want. At the same time,
		  // we need to keep track of previous matches
		  SyntaxTree *prevmatch = straddleTreesPolicy->matched();
		  res = straddleTreesPolicy->nodeEnforce(nst);
		  if (res == NULL) {
			straddleTreesPolicy->matched(this, (*child_)[i-1], &cst);
		  }
		  else {
			straddleTreesPolicy->matched(prevmatch);
		  };
		  dbg(cout << "\nSTRADDLE " << ((res == NULL) ? "(Y)" : "(N)") << " `";
			  child(i-1)->printall(cout,1); cout << "' `";
			  child(i)->printall(cout,1); cout << "'\n");

		}
		//delete nst;
	  }
	}
  }

  if (nc == 0)
	*ltm = lmtc;
  *rtm = rtm1;
  return isSubTreeTainted();
}

class SynTree_LT {
 public:
  bool operator()(const SyntaxTree* s1, const SyntaxTree* s2) {
	return lessSTree(s1, s2);
  }
};

void SyntaxTree::
sortUniqueChildren() {
  int i, j, k;
  sort(child_->begin(), child_->end(), SynTree_LT());
  for (i=0, k=0; (k < child_->size());) {
	// i<=k<size; no dups elems[0:i-1]; elem[i-1]!=elem[k], elem[k] unproc'd
	for (j=k+1; ((j < child_->size())&&((*child_)[k] == (*child_)[j]));) {
	  cout << "\nCOMPRESS\n";
	  j++; //Inv: i<=k<j<=size, elem[k] to [j-1] equal
	}
	//Inv: (elem[j]!=elem[k]), i<=k<j<=size; nodups in 0:i-1;
	// elem[i-1]!=elem[k], elem[k] unproc'd; elem[k:j-1] equal
	(*child_)[i] = (*child_)[k];
	i++; k = j;
	// i<=k<=size; no dups in 0:i-1; elem[i-1]!=elem[k], elem[k] unproc'd
  }
  for (; (i < child_->size()); i++)
	child_->pop_back();
}

SyntaxTree* SyntaxTree::
dagify(SyntaxTreeTab& tab) {
  int nc = numChildren();
  dbg(nTotal++);
  for (int i=0; i < nc; i++) {
	SyntaxTree *st = child(i)->dagify(tab);
	if (loglevel == 100) {
	  cout << "DGF: " << (int)st; st->print(cout); cout << endl;
	}
	(*child_)[i] = st;
  }
  if (nodeType() == OUTPUT_NODE)
	sortUniqueChildren();
  SyntaxTreeTab::iterator it = tab.find(this);
  if (it != tab.end()) {
	dbg(nShared++);
	return it->second;
  }
  else {
	tab[this] = this;
	return this;
  }
}

SyntaxTree* SyntaxTree::
shallowCopy() const {
  SyntaxTree *rv = new SyntaxTree(nodeType(), val_, vlen_, confidence_);
  rv->isSubTreeTainted(isSubTreeTainted());
  rv->isParameterized(isParameterized());
  rv->taintInfo_ = taintInfo_;
  if (child_ != NULL) {
	int nc = numChildren();
	rv->child_ = new vector<SyntaxTree*>(nc);
	for (int i=0; i < nc; i++)
	  (*rv->child_)[i] = (SyntaxTree*)child(i);
  }
  return rv;
}

SyntaxTree* SyntaxTree::
replaceChildren(SyntaxTree* st1, SyntaxTree* st2) const {
  int nc = 2;
  if (st2 == NULL) nc = 1;
  SyntaxTree *rv = new SyntaxTree(nodeType(), val_, vlen_, confidence_);
  rv->isSubTreeTainted(isSubTreeTainted());
  rv->isParameterized(isParameterized());
  rv->taintInfo_ = taintInfo_;
  rv->child_ = new vector<SyntaxTree*>(nc);
  (*rv->child_)[0] = st1;
  if (st2 != NULL)
	(*rv->child_)[1] = st2;
  return rv;
}

SyntaxTree* SyntaxTree::
deepCopy() const {
  SyntaxTree *rv = shallowCopy();
  if ((!isParameterized()) && (taintInfo_ != NULL)) {
	rv->taintInfo_ = new TaintInfo();
	memcpy(rv->taintInfo_, taintInfo_, sizeof(TaintInfo));
  }
  if ((isParameterized()) && (paramInfo_ != NULL)) {
	rv->paramInfo_ = new ParamInfo();
	memcpy(rv->paramInfo_, paramInfo_, sizeof(ParamInfo));
  }
  if (child_ != NULL) {
	int nc = numChildren();
	for (int i=0; i < nc; i++)
	  (*rv->child_)[i] = child(i)->deepCopy();
  }
  return rv;
}

// @@@@ Does not yet do any thing to combine edits
SyntaxTree* SyntaxTree::
maxCommPrefix(const SyntaxTree& st, u_char confThresh) const {
  // Called only on dagified SyntaxTrees
  // @@@@ We shd use tree and string alignment algorithms as a
  // @@@@ basis for computing the common parts. That would allow
  // @@@@ more intelligent summarization, accounting for missing
  // @@@@ words or subtrees.

  SyntaxTree* rv = NULL;
  bool closeMatch = false;
  if (this == &st)
	return (SyntaxTree*)this;

  if ((nodeType() == OUTPUT_NODE) || (st.nodeType() == OUTPUT_NODE)) {
	assert((nodeType() == OUTPUT_NODE) && (st.nodeType() == OUTPUT_NODE));
	SyntaxTree *rv = shallowCopy();
	rv->merge(&st);
	return rv;
  };

  if ((val_ == NULL) ||
	  ((st.val_ != NULL) && (st.numChildren() > numChildren())))
	rv = st.shallowCopy();
  else rv = shallowCopy();
  rv->confidence_ = min(confidence_, st.confidence_);

  if (nodeType() != st.nodeType())
	rv->nodeType(MULTIPLE);

  if ((val_ != NULL) && (st.val_ != NULL)) {
	if ((confidence_ > confThresh) && (st.confidence_ > confThresh)) {
	  unsigned i, j, k;
	  if (((isNodeTainted() || st.isNodeTainted()) &&
		   fabs(1-(vlen_+.01)/(st.vlen_+.01)) < 0.2)) {
		char *tp1 = "n1", *tp2 = "n2";
		MatchRes mr;
		if (vlen_ < st.vlen_)
		  mr = p1FastMatch(val_, vlen_, tp1, st.val_, st.vlen_, tp2,
						   0.5, textm);
		else mr = p1FastMatch(st.val_, st.vlen_, tp2, val_, vlen_, tp1,
							  0.5, textm);
		closeMatch = (!mr.elem_.empty());
	  }
	  char *s = (char *)alloca(vlen_ + st.vlen_ + 5);
	  for (i=0, j=0, k=0; ((i < vlen_) && (j < st.vlen_)); i++,j++,k++) {
		if (val_[i] == st.val_[j])
		  s[k] = val_[i];
		else if (textm.normalize(val_[i]) == textm.normalize(st.val_[j]))
		  s[k] = textm.normalize(val_[i]);
		else if (textm.normalize(val_[i]) == textm.OPTIONAL_CHAR) {
		  s[k] = textm.OPTIONAL_CHAR;
		  j--;
		}
		else if (textm.normalize(st.val_[j]) == textm.OPTIONAL_CHAR) {
		  s[k] = textm.OPTIONAL_CHAR;
		  i--;
		}
		else s[k] = textm.WILD_CARD_CHAR;
	  }

	  for (; (i < vlen_); i++, k++)
		s[k] = textm.WILD_CARD_CHAR;
	  for (; (j < st.vlen_); j++, k++)
		s[k] = textm.WILD_CARD_CHAR;

	  s[k]='\0';
	  rv->val_ = strdup(s);
	  rv->vlen_ = k;
	}
	else {
	  rv->val_ = NULL;
	  rv->vlen_ = 0;
	}
  }

  if ((confidence_ > confThresh) && (st.confidence_ > confThresh)) {
	if (!isNodeTainted()) /*(taintInfo_->begin_ == NULL)*/ {
	  if ((st.isNodeTainted())) {
		if (closeMatch) {
		  rv->taintInfo_ = NULL;
		}
		else {
		  if (loglevel > LOG_DEBUG) { // not >=
			cout<<"**** ERROR **** Merging terms with inconsistent taint\n";
			cout << "\nMerging `";
			print(cout, 1, 10, 0);
			cout << "'\nwith `";
			st.print(cout, 1, 10, 0);
			cout << "'\n";
		  }

		  rv->taintInfo_ = new TaintInfo();
		  memcpy(rv->taintInfo_, st.taintInfo_, sizeof(TaintInfo));
		  if (/*(st.taintInfo_->len_ > 0) && */
			  ((st.taintInfo_->begin_ - st.val_) < rv->vlen_)) {
			rv->taintInfo_->begin_=rv->val_+(st.taintInfo_->begin_-st.val_);
			rv->taintInfo_->len_ = min((rv->val_+rv->vlen_-rv->taintInfo_->begin_), st.taintInfo_->len_);
		  }
		}
	  }
	}
	else if (!st.isNodeTainted()) /*(st.taintInfo_->begin_ == NULL)*/ {
	  if (closeMatch) {
		rv->taintInfo_ = NULL;
	  }
	  else {
		if (loglevel > LOG_DEBUG) { // Not >=
		  cout << "**** ERROR **** Merging terms with inconsistent taint\n";
		  cout << "\nMerging `";
		  print(cout, 1, 10, 0);
		  cout << "'\nwith `";
		  st.print(cout, 1, 10, 0);
		  cout << "'\n";
		}

		rv->taintInfo_ = new TaintInfo();
		memcpy(rv->taintInfo_, taintInfo_, sizeof(TaintInfo));
		if (/*(taintInfo_->len_ > 0) &&*/
			((taintInfo_->begin_ - val_) < rv->vlen_)) {
		  rv->taintInfo_->begin_ = rv->val_ + (taintInfo_->begin_-val_);
		  rv->taintInfo_->len_ = min((rv->val_+rv->vlen_-rv->taintInfo_->begin_), taintInfo_->len_);
		}
	  }
	}
	else {
	  rv->taintInfo_ = new TaintInfo();
	  memcpy(rv->taintInfo_, taintInfo_, sizeof(TaintInfo));
	  int taintBeg = min((taintInfo_->begin_-val_),
						 (st.taintInfo_->begin_-st.val_));
	  rv->taintInfo_->begin_ = min(taintBeg, rv->vlen_)+rv->val_;
	  int taintEnd = max((taintInfo_->begin_-val_)+taintInfo_->len_,
						 (st.taintInfo_->begin_-st.val_)+st.taintInfo_->len_);
	  rv->taintInfo_->len_ = taintEnd - (rv->taintInfo_->begin_-rv->val_);
	}
  }
  else {
	// @@@@ Need to check: we are ignoring low confidence nodes ...
	//rv->isTainted_ = rv->isSubTreeTainted_ = false;
	rv->taintInfo_ = NULL;
  }

  //if (rv->isTainted_ != (rv->taintInfo_ != NULL)) {
  //  cout << "ERROR\n";
  //}
  assert(!rv->isNodeTainted() ||
		 //(rv->taintInfo_ == NULL) || (rv->taintInfo_->len_ == 0) ||
		 ((rv->taintInfo_->begin_ >= rv->val_) &&
		  (rv->taintInfo_->begin_+rv->taintInfo_->len_
		   <= rv->val_+rv->vlen_)));

  int n = numChildren();
  int m = st.numChildren();
  int k = max(n,m);
  if (rv->child_ == NULL)
	rv->child_ = new vector<SyntaxTree*>(k);
  else
	for (int i = rv->numChildren(); i < k; i++)
	  rv->child_->push_back(NULL);
  assert(rv->numChildren() == k);
  for (int j=0; j < k; j++) {
	if (j >= n)
	  (*rv->child_)[j] = (SyntaxTree*)st.child(j);
	else if (j >= m)
	  (*rv->child_)[j] = (SyntaxTree*)child(j);
	else {
	  const SyntaxTree *tc = child(j);
	  const SyntaxTree &stc = *st.child(j);
	  SyntaxTree* rvc = tc->maxCommPrefix(stc);
	  (*(rv->child_))[j] = rvc;
	}
  }

  if (rv == NULL)
	return (SyntaxTree*)this;
  else return rv;
}

int SyntaxTree::
match(const SyntaxTree& st, u_char confThresh) {
  int i, j, rv=0;

  if ((nodeType() != st.nodeType())&&(st.nodeType() != MULTIPLE))
	return false;

	if ((val_!= NULL) && (st.val_ != NULL)) {
	  if ((st.confidence_ > confThresh) && (confidence_ > confThresh)) {
		for (i=0, j=0; ((i < vlen_) && (j < st.vlen_)); i++, j++) {
		  if (val_[i] == st.val_[j]) continue;
		  else if (textm.normalize(val_[i]) == textm.normalize(st.val_[j]))
			continue;
		  else if (textm.normalize(val_[i]) == textm.OPTIONAL_CHAR)
			j--;
		  else if (textm.normalize(st.val_[j]) == textm.OPTIONAL_CHAR)
			i--;
		  else if ((val_[i] == textm.WILD_CARD_CHAR) ||
				   (st.val_[j] == textm.WILD_CARD_CHAR)) continue;
		  else if (st.isNodeTainted()) /*(st.taintInfo_->begin_ != NULL)*/
			break;
		  else {
			if (loglevel >= LOG_DEBUG) {
			  cout << "**** ERROR **** Symbol mismatch while matching `";
			  cout.write(val_, vlen_); cout << "' with pattern '";
			  cout.write(st.val_, st.vlen_); cout << "'\n";
			}
			return SYM_MISMATCH;
		  }
		}
		//@@@@ NOTE: we are matching only until the end of one of the symbols
	  }

	  if (st.isNodeTainted() /*&& (st.taintInfo_->len_ > 0)*/) {
	   // @@@@ We should really use a regular expression match to mark out
	   // @@@@ the tainted components of a tainted node. We have cut some
	   // @@@@ corners at the moment, making a crude (undocumented) assumption
	   // @@@@ that we've implicitly learnt a common untainted prefix, and that
	   // @@@@ in the absence of additional info, we must conservatively assume
	   // @@@@ that every thing to the right is tainted. The tainted portion
	   // @@@@ starts at the same point as in st, or if a mismatch occurs
	   // @@@@ earlier, then at the earlier position.

		if ((st.taintInfo_->begin_ - st.val_) < vlen_) {
		  if (taintInfo_ == NULL)
			taintInfo_ = new TaintInfo();
		  //isTainted_ = isSubTreeTainted_ = true;
		  taintInfo_->begin_ = val_ + (st.taintInfo_->begin_ - st.val_);
		  taintInfo_->len_ = min((val_+vlen_-taintInfo_->begin_), st.taintInfo_->len_);
		}
	  }
	}

  //assert(isTainted_ == (taintInfo_ != NULL));
  assert(!isNodeTainted() ||
		 //(taintInfo_ == NULL) || (taintInfo_->len_ > 0) ||
		   ((taintInfo_->begin_ >= val_) &&
			(taintInfo_->begin_+taintInfo_->len_
			 <= val_+vlen_)));

  if (numChildren() > 0) {
	int j = min(numChildren(), st.numChildren());
	for (i=0; (i<j); i++) {
	  int rv1 = (*child_)[i]->match(*((*st.child_)[i]));
	  rv = min(rv, rv1);
	}
	if (j < numChildren()) {
	  if (loglevel >= LOG_DEBUG)
		cout << "\n**** Warning ***** Expecting " << j << " children, but saw " << numChildren() << " in match(2)\n";
	  dbg(cout << "THIS = ";
		  print(cout, 1, 3, 0);
		  cout << "\n\nST = ";
		  st.print(cout, 1, 3, 0));
	  return UNEXPECTED_SUBTREE;
	}
  }

  return rv;
}

SyntaxTree* SyntaxTree::
abstract() const {
  assert(isParameterized() == false);
  SyntaxTree *rv = new SyntaxTree(nodeType(), val_, vlen_, confidence_);
  rv->isSubTreeTainted(false);
  rv->isParameterized(true);

  /* Check that:
     (a) an input is present in entirety, or a certain pre/suffix (identified
         by some transformation, and of sufficient length)
     (b) may need to introduce a concept of confidence on the taintedness
         and/or transformation: with small confidence on tainting (due to
		 short length), we may still be able to increase confidence by
		 observing a certain flow across different requests.
	 (c)
  */

  if ((taintFrac() > 0.5) && (taintInfo_->len_ >= wordTaintThreshLen)) {
	rv->paramInfo_ = new ParamInfo();
	rv->paramInfo_->paramName_ = taintInfo_->srcNm_;

	unsigned prefixLen = taintInfo_->begin_ - val_;
	rv->paramInfo_->constPrefix_ = new char[prefixLen+1];
	strncpy(rv->paramInfo_->constPrefix_, val_, prefixLen);
	rv->paramInfo_->constPrefix_[prefixLen] = '\0';

	unsigned suffixLen = vlen_ - taintInfo_->len_ - prefixLen;
	rv->paramInfo_->constSuffix_ = new char[suffixLen+1];
	strncpy(rv->paramInfo_->constSuffix_, &val_[vlen_-suffixLen], suffixLen);
	rv->paramInfo_->constSuffix_[suffixLen] = '\0';

	rv->paramInfo_->inputEdits_ = NULL; //@@@@ For now ...

	rv->val_ = NULL; rv->vlen_ = 0;
  }
  else   rv->paramInfo_ = NULL;

  int nc = numChildren();
  if (nc > 0) {
	rv->child_ = new vector<SyntaxTree*>(nc);
	for (int i=0; i < nc; i++)
	  (*rv->child_)[i] = child(i)->abstract();
  }
  else rv->child_ = NULL;

  return rv;
}

void SyntaxTree::
merge(const SyntaxTree *st) {
  assert ((isParameterized()) && (st->isParameterized()) &&
		  (nodeType() == OUTPUT_NODE) && (st->nodeType() == OUTPUT_NODE));
  append(*st->child_);
  sortUniqueChildren();
}
