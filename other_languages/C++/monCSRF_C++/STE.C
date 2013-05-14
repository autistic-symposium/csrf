#include <algorithm>
#include "STE.h"
#include "ST.h"
#include "SyntaxTree.h"
#include "TreeAuto.h"

static const char* const kindtag[] = {
  "UNKNOWN_KIND",
  "CONTEXT_KIND",
  "INPUT_PARAM_KIND",
  "OUTPUT_PARAM_KIND",
  "GLOBAL_KIND",
  "ERROR_KIND"
};

const char* 
kindTag(Kind k){
  if ((k >= 0) && (k < ERROR_KIND))
    return kindtag[k];
  else return kindtag[ERROR_KIND];
}

SymTabEntry::SymTabEntry(const char* name, Kind k) {
  index_ = -1;
  name_ = strdup(name);
  kind_ = k;
  st_ = NULL;
  next_ = prev_ = NULL;
  accCount_ = 0;
  multiMatchCount_ = 0;
  distance_ = 0;
}

bool 
SymTabEntry::operator==(const SymTabEntry& ste) const {
  // @@@@ Reference comparison implemented, but is this appropriate?
  // Probably: STE cannot be copied or assigned, and there isn't much
  // reason to create multiple STEs for the same symbol, so this is enough
  return (&ste == this);
}

void 
SymTabEntry::print(ostream& os, int indent, bool sort) const {
  os << name() << ": count=" << accCount_; 
  if (distance_ > 1.0E-8)
	os << ", dist=" << (distance_/accCount_);
  if (multiMatchCount_ > 0) os << ", multiMatches=" << multiMatchCount_;

  if (st_ != NULL) 
	st_->printST(os, indent, sort);
}

void
SymTabEntry::buildAuto() {
  if (st_ != NULL)
	st_->buildAuto();
}

 /***************************************************************************/

OutParam::OutParam(const char* name):
  SymTabEntry(name, OUTPUT_PARAM_KIND)  {
  outs_ = inouts_ = inoutAtks_ = NULL;
  inAuto_ = outAuto_ = NULL;
}

bool 
OutParam::operator==(const OutParam& ste)const {
  // @@@@ Reference comparison implemented, but is this appropriate?
  // Probably: STE cannot be copied or assigned, and there isn't much
  // reason to create multiple STEs for the same symbol, so this is enough
  return (&ste == this);
}

void
prtPatSet(PatSet *p, ostream &os, unsigned indent) {
  if (p != NULL) {
	PatSet::iterator pi;
	for (pi = p->begin(); pi != p->end(); pi++) {
	  SyntaxTree *st = *pi;
	  if (st != NULL) {
		os << endl;
		st->print(os, 0, 1000000, 50, indent+2);
		os << endl;
	  }
	}
  }
}

void 
OutParam::print(ostream& os, int indent, bool sort) const {
  SymTabEntry::print(os, indent, sort);
  prtPatSet(outs_, os, indent);
  if (outAuto_ != NULL) {
	os << "\n---------------------------- AUTOMATON ----------------------\n";
	outAuto_->print(os, indent);
	os << "---------------------------- AUTOMATON END ------------------\n";
  }
#ifdef DBG_INPFILT
  prtPatSet(inouts_, os, indent);
  if (inAuto_ != NULL) {
	os << "\n-------------------------- IN AUTOMATON ----------------------\n";
	inAuto_->print(os, indent);
	os << "-------------------------- IN AUTOMATON END ------------------\n";
  }
#endif
}

void
OutParam::buildAuto() {
  if ((outs_ != NULL) && (!outs_->empty()))
	  outAuto_ = new TreeAuto(*outs_);
  if ((inouts_ != NULL) && (!inouts_->empty()))
	  inAuto_ = new TreeAuto(*inouts_);
  SymTabEntry::buildAuto();
}

 /***************************************************************************/

void
Session::insertIn(SymTabEntry& ste, const char *val) {
  int i = ste.index();
  assert (ste.kind() == INPUT_PARAM_KIND);
  while (i > inval_.size())
	inval_.push_back(NULL);
  if (i == inval_.size())
	inval_.push_back(val);
  else {
	if (inval_[i] != NULL) {
	  cout << "WARNING: Updating input sym: " << &ste;
	  cout << " OLD VAL=" << inval_[i] << " NEW VAL=" << val << endl;
	}
	inval_[i] = val;
  }
}

void
Session::insertOut(OutParam& op, SyntaxTree *st) {
  int i = op.index();
  while (i >= out_.size())
	out_.push_back(NULL);
  if (out_[i] == NULL)
	out_[i] = new vector<SyntaxTree*>(); 
  out_[i]->push_back(st);
}

/***************************************************************************/

Context::Context(const char* name):
  SymTabEntry(name, CONTEXT_KIND), inputNm_(), outputNm_(), ses_() {
  outSt_ = NULL;
  curSes_ = NULL;
  nin_ = nout_ = 0;
}

void 
Context::newsession() { 
  curSes_ = new Session(*this, nin_, nout_); 
  ses_.push_back(curSes_); 
};


bool 
Context::operator==(const Context& ste)const {
  // @@@@ Reference comparison implemented, but is this appropriate?
  // Probably: STE cannot be copied or assigned, and there isn't much
  // reason to create multiple STEs for the same symbol, so this is enough
  return (&ste == this);
}

void 
Context::print(ostream& os, int indent, bool sort) const {
  SymTabEntry::print(os, indent, sort);
  if (outSt_ != NULL) 
	outSt_->printST(os, indent, sort);
}

void
Context::buildAuto() {
  SymTabEntry::buildAuto();
  if (outSt_) {
	for (SymTab::iterator it = outSt_->begin(); (it != outSt_->end()); ++it) {
	  OutParam* op = (OutParam*)*it;
	  const char* nm = op->name();
	  if ((strcmp(nm, "popen") == 0) || (strstr(nm, "sql") != NULL)) {
		int i = op->index();
		PatSet* positiveEx = new PatSet();
		PatSet* negativeEx = new PatSet();
		for (int j=0; j < ses_.size(); j++) {
		  Session *s = ses_[j];
		  vector<SyntaxTree*>* out = s->outputval(i);
		  if (out != NULL) {
			SyntaxTree *st = new SyntaxTree(GRP);
			for (int k=0 ; k < s->ninputs(); k++) {
			  char *ss = s->inputval(k);
			  if (ss == NULL) ss = "_NP_"; // Not present
			  unsigned sslen = strlen(ss);
			  if (sslen < 200) { // @@@@ hack 
				SyntaxTree* st1 = new SyntaxTree(INPUT_NODE, ss, sslen);
				st->addChild(st1);
			  }
			}

			SyntaxTree* st2 = new SyntaxTree(OUTPUT_NODE);
			for (vector<SyntaxTree*>::iterator it = out->begin();
				 it != out->end(); ++it) {
			  SyntaxTree* stt = *it;
			  assert(stt->nodeType()==ROOT_NODE);
			  stt = stt->shallowCopy();
			  stt->nodeType(GRP);
			  st2->addChild(stt);
			}
			SyntaxTree *st3 = new SyntaxTree(ROOT_NODE, st, st2);
			SyntaxTree *st4 = st3->abstract();
			//cout << "XXXXX";
			//st4->print(cout);
			if (s->attacksFound())
			  negativeEx->push_back(st4);
			else positiveEx->push_back(st4);
		  }
		}

		op->inOuts(positiveEx);
		op->inOutAtks(negativeEx);
	  }
	  op->buildAuto();
	}
  }
}
