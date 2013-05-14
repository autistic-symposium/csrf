#include <algorithm>

#include <math.h>
#include "TreeAuto.h"

const Position null_pos(Position::MAX_PARENT+1,1);
const Position root_pos(Position::MAX_PARENT+1,0);

void printst(SyntaxTree* st) {
  st->print(cout);
  cout << endl; cout.flush();
};

void prtSTSet(const STSet& sts) {
  cout << "STSET {\n";
  for (STSet::const_iterator it = sts.begin(); it != sts.end(); it++) {
	cout << (int)(*it) << ": ";
	(*it)->print(cout);
	cout << endl;
  }
  cout << "}\n";
  cout << "DAGIFYING\n";
  SyntaxTreeTab stt;
  loglevel = 100;
  for (STSet::const_iterator it = sts.begin(); it != sts.end(); it++) {
	SyntaxTree *st = *it;
	if (st->dagify(stt) == st)
	  cout << " NA";
	else cout << " A";
  }
  cout << endl;
  loglevel = 2;
}

void printFringe(const Fringe* r) {
  if ((r != NULL) && (r->begin() != r->end()))
	for (Fringe::const_iterator fi = r->begin(); fi != r->end(); fi++) {
	  fi->second->print(); cout << "@"; fi->first.print(); cout << ", ";
	}
  else cout << "NULL fringe, ";
};

void printFL(const FringeList* r) {
  if (r != NULL)
	for (FringeList::const_iterator fli = r->begin(); fli != r->end(); fli++) {
	  printFringe(*fli); cout << endl;
	}
  else cout << "NULL FL\n";
};

int nShared, nTotal, nCumShared, nCumTotal, nTaintSharProb;

/*
  while merging I/O, consider the following:
  (a) merge outputs (set union) when inputs are identical
  (b) merge inputs using maxCommPrefix when outputs are identical: NO NEED:
       this merging will happen automatically when the automaton is built.
  (c) if one output is a superset of another output, consider
      splitting the first into two I/O pairs such that (b) becomes applicable.
*/

TreeAuto::TreeAuto(PatSet& ps) {
  PatSet psn;
  PatSet::iterator psi, psi1;
  FringeList next;

  // Use dagify to discover duplicates in ps, eliminate them.
  // Duplicate discovery happens at internal nodes as well, so that
  // equal subtrees will be identified, and a single physical copy shared.

  dbg(nCumShared=0; nCumTotal=0; nTaintSharProb=0);
  SyntaxTreeTab stt;
  for (psi = ps.begin(); (psi != ps.end()); psi++) {
	SyntaxTree *st = *psi;
	dbg(nTotal=0; nShared = 0);
	if (st->dagify(stt) == st) {
	  dbg(cout << "\n>>>>Dagified: "; st->print(cout,true,30,0); cout << endl);
	  psn.push_front(st);
	}
    dbg(cout << "\n>>>>Shared " << nShared << " of " << nTotal << " nodes\n";
		nCumShared+=nShared; nCumTotal+= nTotal);
  }

  dbg(cout << "\n>>>>SHARED " << nCumShared <<" of "
	  <<nCumTotal<< " nodes, probs = " << nTaintSharProb << "\n");

  bool ioauto = psn.front()->isParameterized();
  if (ioauto) {
	vector<SyntaxTree*> psn1;
#ifdef DBG_INPFILT
	prtPatSet(&psn, cout);
#endif
	psi = psn.begin();
	psn1.push_back(*psi);
	psi++;
	for (; (psi != psn.end()); psi++) {
	  SyntaxTree *st = *psi;
	  bool merged = false;
	  for (int i=0; (i < psn1.size()); i++) {
		if (psn1[i]->child(0) == (st->child(0))) { // identical inputs
		  cout << "MERGED "; st->print(cout);
		  cout << "\n WITH "; psn1[i]->print(cout);
		  psn1[i] = psn1[i]->shallowCopy();
		  psn1[i]->child(1, psn1[i]->child(1)->shallowCopy());
		  psn1[i]->child(1)->merge(st->child(1));
		  psn1[i] = psn1[i]->dagify(stt);
		  cout <<"\n RESULT "; psn1[i]->print(cout);
		  merged = true;
		  break;
		}
	  }
	  if (!merged)
		psn1.push_back(st);
	}

	for (vector<SyntaxTree*>::iterator psi = psn1.begin();
		 (psi != psn1.end()); psi++) {
	  SyntaxTree* st = *psi;
	  if (st != NULL) {
		Fringe *n = new Fringe;
		(*n)[root_pos] = st;
		next.push_front(n);
	  }
	}
  }
  else {
	for (psi = psn.begin(); (psi != psn.end()); psi++) {
	  SyntaxTree* st = *psi;
	  if (st != NULL) {
		Fringe *n = new Fringe;
		(*n)[root_pos] = st;
		next.push_front(n);
	  }
	}
  }

  buildAuto(0, psn, next, !psn.front()->isParameterized());

#ifdef DBG_INPFILT
  if (ioauto) {
	cout << "\n------------------------ IN AUTOMATON ----------------------\n";
	print(cout);
	cout << "-------------------------- IN AUTOMATON END ------------------\n";
  }
#endif

  dbg(cout << "\n>>>>Constructed automaton: "; print(); cout << endl);
}

TreeAuto::TreeAuto(int d, PatSet& ps, FringeList& next, bool fastButSloppy) {
  buildAuto(d, ps, next, fastButSloppy);
}

#define SUIT_THR 216 // approx. 0.85
#define THR_FAC 1
#define THR_INC 1
#define THR_FUNC(x) (round(THR_FAC * log((double)(x)))+THR_INC)
#define THRESH 33 // approx. 0.13
#define EPSILON 0.13

class MeritInfo {
public:
  u_char confidence_;
  int numPats_;
  int numBad_; // Bad position (tainted or output) for this many patterns
  int numSubtreeTainted_;
  STNVSet  syms_;
  STSet  vals_;
public:
  MeritInfo(): syms_(), vals_() {
	confidence_ = 255;
	numPats_ = numBad_ = numSubtreeTainted_ = 0;
  };
};

size_t hashPos(const Position p) { return p.hash(); };
bool lessPos(const Position v1, const Position v2)
   {return (v1.occursBefore(v2));};
bool eqPos(const Position v1, const Position v2) { return (v1 == v2);};
HashCmpClass(Pos, const Position, hashPos, eqPos, lessPos);
typedef HashMap(Pos, Position, MeritInfo) PosMeritMap;

void printPMI(const PosMeritMap& pmm) {
  for (PosMeritMap::const_iterator pmmi = pmm.begin();
	   pmmi != pmm.end(); pmmi++) {
	Position p = pmmi->first;
	MeritInfo m = pmmi->second;
	cout << "@"; p.print(); cout << ": ";
	cout << "n=" << m.numPats_ << " conf=" << (int)m.confidence_
		 << " nT=" << m.numBad_ << " nST="
		 << m.numSubtreeTainted_ << " nvals=" << m.vals_.size()
		 << " nsyms=" << m.syms_.size() << endl;
  }
}

static inline void
insert(PosMeritMap& pmm, Position p, SyntaxTree& subpat) {
  MeritInfo& mi = pmm[p];
  mi.confidence_ = min(mi.confidence_, subpat.confidence());
  mi.numPats_++;
  if ((subpat.isNodeTainted()) || (subpat.nodeType() == OUTPUT_NODE))
	mi.numBad_++;
  if (subpat.isSubTreeTainted())
	mi.numSubtreeTainted_++;
  if  (subpat.nodeType() != OUTPUT_NODE) {
	// we want to pretend that number of distinct values is zero for
	// output nodes. This will ensure that it never gets picked.
	mi.vals_.insert(&subpat);   autoresize(mi.vals_);
  }
  mi.syms_.insert(&subpat);   autoresize(mi.syms_);
}

static inline bool goodenough(PosMeritMap& pmm, Position p, int n) {
  // @@@@ We shd use something like consistent_taint here to stop
  // @@@@ creating unnecessary branches. But consistent_taint can
  // @@@@ work only after all positions across all SyntaxTrees are
  // @@@@ examined --- an expensive task that directly counters the
  // @@@@ purpose of "goodenough", namely, to pick the first possible
  // @@@@ branch if it will do.

  MeritInfo& mi = pmm[p];
  /*cout << "\n*** nvals=" << mi.vals_.size() << ", npats=" << mi.numPats_
	   << ", nwithchildren=" << mi.numWithChildren_ << ", ntaint="
	   << mi.numBad_ << "at pos ";
	   p->print(cout);*/

#define usefultest(mi) \
((mi.vals_.size()>1) && (mi.numBad_ == 0))

  if (usefultest(mi) && (n == mi.numPats_) && (mi.confidence_ >= SUIT_THR))
	if (mi.syms_.size() <= THR_FUNC(n))
	  return true;
	else {
	  dbg(
		cout << "**** WARNING **** test rejected due to branching factor: n="
			 << n << " THR_FUNC(n)=" <<  THR_FUNC(n)
			 << ", branches=" << mi.vals_.size() << endl);
	};
  return false;
}

#define comparable(a,b) ((a > b) ? ((a-b) < THRESH) : ((b-a) < THRESH))
#define adequate(a,b) ((a > 50) == (b > 50))

static inline bool lessCost(const MeritInfo& mi1, const MeritInfo& mi2,
							Position p1, Position p2) {
 if ((mi1.vals_.size() > 1) == (mi2.vals_.size() > 1))
  if (mi1.numPats_ == mi2.numPats_)                    //index next
     if (mi1.numBad_ == mi2.numBad_)                   //node taint next
	  if (comparable(mi1.confidence_,mi2.confidence_)) //confidence next
		if ((mi1.syms_.size() > 1) == (mi2.syms_.size() > 1))
		 // next, prefer positions that create more than a one-way branch
		 if (fabs(1-(((float)mi1.syms_.size())/mi2.syms_.size()))<EPSILON)
		 // next prefer low branching factor
		   if (mi1.numSubtreeTainted_ == mi2.numSubtreeTainted_)
			 //next, prefer positions with tainted subtree
			 return (p1.occursBefore(p2)); // finally, prefer left-most
		   else return (mi1.numSubtreeTainted_ > mi2.numSubtreeTainted_);
		 else return (mi1.syms_.size() < mi2.syms_.size());
		else return (mi1.syms_.size() > 1);
	  else return (mi1.confidence_ > mi2.confidence_);
	 else return (mi1.numBad_ < mi2.numBad_);
  else return (mi1.numPats_ > mi2.numPats_);
 else return (mi1.vals_.size() > 1);
}

#define consistent_taint(mi, n) \
   ((n == 1) || (mi->numSubtreeTainted_ == 0))
/* Conservative: subtrees may be consistently tainted, but not recognized */

/* @@@@%% Note: we are using consistent_taint to stop further decision */
/* @@@@%% tree construction as soon as possible. We can do this if the */
/* @@@@%% remaining patterns are very similar. Being very similar      */
/* @@@@%% requires that the patterns be consistently tainted, but this */
/* @@@@%% is not enough.                                               */
static inline Position
selectMinCost(PosMeritMap& pmm, int n, bool& allConsistentlyTainted,
			  bool& rvIsBad, bool& index, u_char& conf) {
  Position rv = null_pos;
  MeritInfo* rmi=NULL;
  PosMeritMap::iterator pmmi = pmm.begin();
  allConsistentlyTainted=true;
  rvIsBad=false; // Bad = tainted or output
  if (pmmi != pmm.end()) {
	rmi = &pmmi->second;
	rv = pmmi->first;
	rvIsBad = (rmi->numBad_ > 0);
	allConsistentlyTainted = consistent_taint(rmi, n);
	for (pmmi++; (pmmi != pmm.end()); pmmi++) {
	  // @@@@ iterating over pmmi is nondeterministic, since order of
	  // @@@@ elements in hashmap is arbitrary; has no relation to insert order
	  // @@@@ May change when object sizes change? It can surely change the
	  // @@@@ buckets into which each object hashes based on its address.
	  Position p = pmmi->first;
	  MeritInfo *mi = &pmmi->second;
	  allConsistentlyTainted = allConsistentlyTainted&&consistent_taint(mi,n);
	  if (lessCost(*mi, *rmi, p, rv)) {
		rmi = mi;
		rvIsBad = (rmi->numBad_ > 0);
		rv = p;
	  }
	}
  }
  index = (rmi != NULL) && (rmi->numPats_ == n);
  conf = rmi->confidence_;
  return rv;
}

Position TreeAuto::
select(FringeList& next, bool fastButSloppy) {
  PosMeritMap pmm;
  STSet outputs(16);
  bool inouts = false;
  FringeList::iterator next_i = next.begin();
  assert(next_i != next.end());
  int i=next.size();
  for (next_i = next.begin(); (next_i != next.end()); next_i++) {
	Fringe::iterator next_ij;
	for (next_ij=(*next_i)->begin();(next_ij!=(*next_i)->end()); next_ij++) {
	  Position p = next_ij->first;
	  SyntaxTree* st = next_ij->second;
	  if (st->isParameterized()) inouts=true;
	  if (st->nodeType() == OUTPUT_NODE) outputs.insert(st);
	  if (pmm.count(p) == 0) { // Have not processed this position before ...
		FringeList::iterator next_k = next_i;
		for (next_k; (next_k != next.end()); next_k++) {
		  Fringe::iterator n = (*next_k)->find(p);
		  if (n != (*next_k)->end()) {
			SyntaxTree *subpat = n->second;
			insert(pmm, p, *subpat);
		  }
		}
		autoresize(pmm);
		if ((fastButSloppy) && goodenough(pmm, p, i))
		  return p;
		else { dbg(cout << "Not goodenough: "; p.print();
				   printPMI(pmm);)}
	  }
	}
  }

  if (outputs.size() == 1) // Done!
	return null_pos;

  //if ((inouts) && (outputs.size() > 0)) prtSTSet(outputs);

  bool rvIsBad; // Bad = tainted or output
  bool index;
  bool allConsistentlyTainted; u_char conf;
  Position rv=selectMinCost(pmm,i, allConsistentlyTainted, rvIsBad,
							index, conf);
  if ((allConsistentlyTainted) && (!inouts))
	return null_pos; // @@@@ shd check if all pats are similar enough to be merged
  if (!index) {
	if (loglevel >= LOG_DEBUG) {
	  cout << "******* Warning ******** No index among " << i << " patterns ...\n";
	  printPMI(pmm);
	  printFL(&next);
	  cout << "<<<< Done (no index)"; rv.print();
	}
	rv = null_pos;
  }
  if (conf < 50) {
	if (loglevel >= LOG_DEBUG) {
	  cout << "**** Warning **** Confidence " <<(unsigned)conf << " too low\n";
	  printPMI(pmm);
	  cout << "<<<< Done (low confidence)"; rv.print();
	}
	rv = null_pos;
  }
  if (rvIsBad) {
	if (loglevel >= LOG_DEBUG)
	  cout<<"******* Warning ******** No untainted positions to branch on ...\n";
    rv = null_pos;
  }
  return rv;
}

class LessSTNV {
public:
  bool operator()(const SyntaxTree* s1, const SyntaxTree* s2) const{
	return (s1->lessNodeVal(s2));
  }
};


void TreeAuto::buildAuto(int depth, PatSet& ps, FringeList& next,
						 bool fastButSloppy) {
  // @@@@ Need to generalize the algorithm so that it can work
  // @@@@ with situations where certain key parameters move around --
  // @@@@ branching on a fixed position does not work well in that
  // @@@@ case as the position of the key argument changes.

  if (depth == 0) {
	dbg(cout << "////////////////////BuildAuto called\n";
	printFL(&next));
  }

  num_ = next.size();
  //if (!fastButSloppy)
  //cout << "y";
  if ((pos_ = select(next, fastButSloppy)) == null_pos) {
	// Can't branch any more, create leaf
	if (!next.empty()) {
	  FringeList::iterator next_i = next.begin();
	  leaf_ = new LeafInfo();
	  for (Fringe::iterator fii = (*next_i)->begin();
		   fii != (*next_i)->end(); fii++)
		(*leaf_)[fii->first] = LeafElem(fii->second);
	  for (++next_i; (next_i != next.end()); next_i++) {
		Fringe* f = *next_i;
		Fringe::iterator fi;
		for (fi = f->begin(); fi != f->end(); fi++) {
		  Position p = fi->first;
		  SyntaxTree *st = fi->second;
		  LeafInfo::iterator fi1 = leaf_->find(p);
		  if (fi1 != leaf_->end()) {
			//(*pat_)[p] = fi1->second->maxCommPrefix(*st);
			fi1->second.st_ = fi1->second.st_->maxCommPrefix(*st);
			fi1->second.num_++;
		  }
		  else (*leaf_)[p] = LeafElem(st);
		}
	  }
	  /*for (Fringe::iterator fi = pat_->begin(); fi != pat_->end(); fi++) {
		fi->second->print(cout,2,true);cout << " @ ";
		printpos(fi->first, cout);cout << endl;
	  }*/
	}
	else {
	  leaf_ = NULL;
	  if (loglevel >= LOG_DEBUG)
		cout << "ERROR: empty automaton leaf\n";
	}
  }
  else if (!next.empty()) { // Non-leaf: branch on pos_, create transitions
	FringeList::iterator next_i;
	map<SyntaxTree*, FringeList*, LessSTNV> transtab;
	for (next_i = next.begin(); (next_i != next.end()); next_i++) {
	  Fringe& f = **next_i;
	  SyntaxTree *st = f.find(pos_)->second;
	  assert(st != NULL);
	  f.erase(pos_);
	  int nc = st->numChildren();
	  for (int i=0; i < nc; i++) {
		Position p(depth, i); //***%%% SEEMS WRONG: 1st param to Position()
		f[p] = st->child(i);
	  }
	  if (f.size() > 0) {
		if (transtab.find(st) == transtab.end())
		  transtab[st] = new FringeList;
		transtab[st]->push_front(&f);
	  }
	}

	map<SyntaxTree*, FringeList*, LessSTNV>::iterator tt_i;
	trans_ = new Transition();
	for (tt_i = transtab.begin(); tt_i != transtab.end(); tt_i++) {
	  SyntaxTree* st = tt_i->first;
	  FringeList* fl = tt_i->second;
	  (*trans_)[st] = new TreeAuto(depth+1, ps, *fl, fastButSloppy);
	}
  }
  else {
	trans_ = NULL;
	if (loglevel >= LOG_DEBUG)
	  cout << "ERROR: empty automaton internal node\n";
  }

  if (depth == 0) {
	dbg(cout << "///////////Done\n";
	print();
	cout << "//////////With BuildAuto\n");
  }
}

typedef pair<SyntaxTree* const, TreeAuto*> TransEntry;
class TR_LT {
 public:
  bool operator()(const TransEntry* s1, const TransEntry* s2) {
	return ((s1 != NULL) && (s2 != NULL) && lessSTNV(s1->first, s2->first));
  }
};

void TreeAuto::print(ostream& os, int indent, const Position& prev) const {
  Transition::const_iterator ti;
  bool no_abbrev=true;
  static int lastNum;
  if ((num_ != 1) && ((prev == null_pos) || (lastNum != num_)))
	os << "[N=" << num_ << "] ";
  lastNum = num_;
  if (pos_ != null_pos) {
	no_abbrev = true /*((prev == NULL) || (prev->parent_ != pos_->parent_) ||
					   (prev->posNum_+1 != pos_->posNum_))*/;
	if (no_abbrev) {
	  os << " : switch ";
	  pos_.print();
	  os << " {\n";
	  printIndent(os, indent+2);
	}
	else if (trans_->size() > 1) {
	  os << endl;
	  printIndent(os, indent+2);
	}
	else os << ' ';
	indent += 2;

	int n = trans_->size();
	if ((no_abbrev) || (n > 1))
	  os << "case ";

	const TransEntry** tre = (const TransEntry**)alloca(n*sizeof(TransEntry*));
	int i;
	for (i=0, ti = trans_->begin(); (ti != trans_->end()); ti++, i++)
	  tre[i] = &(*ti);

    sort(tre, tre+trans_->size(), TR_LT());

	for (int i=0; (i < n); i++) {
	  if (tre[i]->first->vLen() == 0)
		os << getNodeName(tre[i]->first->nodeType());
	  else os.write(tre[i]->first->value(), tre[i]->first->vLen());
	  TreeAuto* d = tre[i]->second;
	  d->print(os, indent, pos_);
	  if (i < n-1)
		os << "case ";
	}

	indent -= 2;
	if (no_abbrev) {
	  os << "}\n";
	  printIndent(os, indent);
	}
  }
  else {
	os << " FINAL {";
	for (LeafInfo::const_iterator ri=leaf_->begin(); (ri!=leaf_->end()); ri++) {
	  ri->second.st_->print(os, 0/*1*/, 500000, 50, indent);
	  if (ri->second.num_ != num_)
		os << " @ [" << ri->second.num_ << ']';
	  else os << " @";
	  ri->first.print();
	  os << ' ';
	}
	os << "}\n";
	printIndent(os, indent);
  }
}

int TreeAuto::enforce(SyntaxTree& st) const {
  vector<SyntaxTree*> r;
  r.push_back(&st);
  return enforce(r, 1);
}

int TreeAuto::enforce(vector<SyntaxTree*>& r, int rem) const {
  int rv=0;
  if (pos_ != null_pos) {
	SyntaxTree *st;
	if (pos_ == root_pos) {
	  st = r[0];
	}
	else {
	  assert(pos_.parent() < r.size());
	  SyntaxTree *par = r[pos_.parent()];
	  if (par->numChildren() <= pos_.child()) {
		if (loglevel >= LOG_DEBUG) {
          cout<<"**** ERROR **** Child " << pos_.child() << " not present\n";
          dbg(cout << "in "; par->print(); cout << "\nAUTO=";
			  print(); cout << endl);
		}
		return POS_NOT_PRESENT;
	  }
	  st = par->child(pos_.child());
	  r.push_back(st);
	}
	Transition::const_iterator ti = trans_->find(st);
	rem = rem-1+st->numChildren();
	if (ti == trans_->end()) {
	  //if (trans_->size() > 1) {
		// @@@@ Shouldn't print an error here: we should continue down
		// @@@@ all branches. After matching is done, check if variation in
		// @@@@ structure is due to tainted data -- if not, there is no
		// @@@@ reason to object due to taint and/or structural deviation

		if (loglevel >= LOG_DEBUG) {
		  cout << "**** ERROR **** No matching transition in decision tree\n";
		  cout << "Matching "; st->print(cout); cout << " @ ";
		  pos_.print(); cout << endl;
		}
		return SYM_MISMATCH;
		//}
   	  /*else {
		ti = trans_->begin();
		if (loglevel >= LOG_WARN) {
		  cout << "**** Warning **** No match for ";
		  cout.write(pat->stn().begin_, pat->stn().len_);
		  cout << " found singleton transition on ";
		  cout.write(ti->first->begin_, ti->first->len_);
		  cout << " taking it. May be need more training?\n";
		}
	  }*/
	}

	TreeAuto *ns = ti->second;
	if (rem > 0)
	  return ns->enforce(r, rem);
	else return 0;
  }
  else if (leaf_ != NULL) { // Leaf node
	LeafInfo::iterator f_i;
	int rv1;
	for (f_i = leaf_->begin(); (f_i != leaf_->end()); f_i++) {
	  Position p = f_i->first;
	  SyntaxTree *psub = f_i->second.st_;
	  SyntaxTree *rsub = NULL;
	  if (p != root_pos) {
		assert(p.parent() < r.size());
		rsub = r[p.parent()];
		if (rsub->numChildren() > p.child()) {
		  rsub = rsub->child(p.child());
		}
		else if (f_i->second.num_ == num_) { // Required subterm
		  if (loglevel >= LOG_DEBUG) {
			cout << "**** ERROR **** Child " << p.child() << " > numChildren("
				 << rsub->numChildren() << ")\n";
            dbg(cout << "IN ";
				rsub->print(cout,1,2,0); cout << endl;
				print();
				//psub->print(cout,1,2,0); cout << endl
				);
		  }
		  return POS_NOT_PRESENT;
		}
		else continue;
	  }
	  else rsub = r[0];
	  if ((rv1 = rsub->match(*psub)) < 0)
		rv=rv1; // @@@@ returns only the last error
	}
  }
  return rv;
}
