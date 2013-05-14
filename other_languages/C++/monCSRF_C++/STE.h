#ifndef STE_H
#define STE_H

#include <list>
#include "ST.h"
#include <assert.h>
#include <iostream>

class SymTabEntry;
class SyntaxTree;
class TreeAuto;
class Context;
class Session;

typedef list<SyntaxTree*> PatSet;
void prtPatSet(PatSet *p, ostream &os, unsigned indent=0);

enum Kind {
  // If this enum is changed, kindTag() needs to be updated correspondingly
  UNKNOWN_KIND,
  CONTEXT_KIND, 
  INPUT_PARAM_KIND,
  OUTPUT_PARAM_KIND,
  GLOBAL_KIND,
  ERROR_KIND
};

class SymTabEntry {
 public:
  /*****************************************************************
	The implementation of this class, as well its descendents.
	makes definite assumtions about the use of kind, name, prev, next
	and the symbol table fields. As such, methods operating on these
	fields are non-virtual. (Overriding them in subclasses is strongly
	discouraged.) All other operations are virtual, whether or not
	we can see any potential use of overriding them.
  ****************************************************************/

  SymTabEntry(const char* name, Kind kind);
  virtual ~SymTabEntry() {};

  int index() const { return index_; };
  void index(int i) { index_ = i; };

  const char* name() const { return name_; }
  void name(const char* str) { name_=str; };

  Kind kind() const { return kind_; } 
  void kind(Kind kind) { kind_ = kind; };

  int accCount() const { return accCount_;};
  void resetAccCount(int nval=0) {accCount_ = nval;};
  void incAccCount(int inc=1) { accCount_+=inc;};

  int multiMatch() const { return multiMatchCount_;};
  void resetMultiMatch(int nval=0) {multiMatchCount_ = nval;};
  void incMultiMatch(int inc=1) { multiMatchCount_+=inc;};

  float dist() const { return distance_;};
  void resetDist(float nval=0.0) {distance_ = nval;};
  void incDist(float inc) { distance_+=inc;};

  virtual const SymTabEntry* lookUp(const char* name) const {
	if (st_ != NULL) return st_->lookUp(name);
	else return NULL;
  }

  virtual SymTabEntry* lookUp(const char* name) {
	if (st_ != NULL) return st_->lookUp(name);
	else return NULL;
  }
  
  const SymTabEntry* prev() const { return prev_ ;};
  const SymTabEntry* next() const  { return next_ ;};

  void prev(SymTabEntry *se) { prev_ = se; }
  void next(SymTabEntry *se) { next_ = se; }

  SymTab* symTab() { return st_; };
  SymTabEntry* prev() { return prev_ ;};
  SymTabEntry* next()  { return next_ ;};

  virtual ErrorST insert(SymTabEntry* ste) {
    if (st_ == NULL) st_ = new SymTab();
    return st_->insert(ste);
  }

  virtual void buildAuto();

  virtual bool operator==(const SymTabEntry&) const;
  virtual bool operator!=(const SymTabEntry& ste) const 
  { return !operator==(ste); };

  virtual void print(ostream& os, int indent=0, bool sort=true) const;

 private:
  int index_;
  const char* name_;
  Kind kind_;
  SymTab* st_;

  int accCount_;
  int multiMatchCount_;
  float distance_;

 private: 
  // These two fields are used to link the STEs so that their order
  // of declaration can be maintained.
  SymTabEntry* next_;
  SymTabEntry* prev_;

 private:
  const SymTabEntry& operator=(const SymTabEntry&); // Disable assignment
  SymTabEntry(const SymTabEntry&);                  // and copy constructor
};
	
inline ostream& operator<<(ostream& out, const SymTabEntry* ste){
  ste->print(out);
  return out;
}

class OutParam: public SymTabEntry {
 public:
  OutParam(const char* name);
  virtual ~OutParam() {};

  TreeAuto *outputAuto() { return outAuto_; }
  const TreeAuto *outputAuto() const { return outAuto_; }
  TreeAuto *inputAuto() { return inAuto_; }
  const TreeAuto *inputAuto() const { return inAuto_; }

  PatSet *outputs() { return outs_; };
  void addChild(SyntaxTree& s) { 
	if (outs_ == NULL) outs_ = new PatSet();
	outs_->push_back(&s); 
  }
  void inOuts(PatSet* p) { inouts_ = p; };
  void inOutAtks(PatSet* p) { inoutAtks_ = p; };

  void buildAuto();

  virtual bool operator==(const OutParam&) const;
  virtual bool operator!=(const OutParam& ste) const 
  { return !operator==(ste); };

  virtual void print(ostream& os, int indent=0, bool sort=true) const;

 private:
  PatSet *outs_;
  PatSet *inouts_;
  PatSet *inoutAtks_;
  TreeAuto *outAuto_;
  TreeAuto *inAuto_;
};

class Context: public SymTabEntry {
 public:
  Context(const char* name);
  virtual ~Context() {};

  Session* cursession() { return curSes_; };
  void newsession();

  unsigned ninputs() const { return nin_;};
  unsigned noutputs() const { return nout_;};
  const char* inputNm(unsigned index) const {
	if (index < nin_) return inputNm_[index]; else return NULL;
  };

  const char* outputNm(unsigned index) const {
	if (index < nout_) return outputNm_[index]; else return NULL;
  };

  ErrorST insert(SymTabEntry* ste) {
	ErrorST rv;
	if (ste->kind() == OUTPUT_PARAM_KIND) {
	  if (outSt_ == NULL) outSt_ = new SymTab();
	  if ((rv = outSt_->insert(ste)) == OK) {
		outputNm_.push_back(ste->name()); nout_++; 
	  }
	}
	else {
	  if ((rv = SymTabEntry::insert(ste)) == OK) {
		inputNm_.push_back(ste->name()); nin_++;
	  }
	}
	return rv;
  }

  SymTabEntry* insertNewIn(const char* nm) {
	SymTabEntry *rv = lookUp(nm);
	if (rv == NULL) {
	  rv = new SymTabEntry(nm, INPUT_PARAM_KIND);
	  insert(rv);
	}
	return rv;
  }

  OutParam* insertNewOut(const char* nm) {
	OutParam *rv = (OutParam*)lookUp(nm);
	if (rv == NULL) {
	  rv = new OutParam(nm);
	  insert(rv);
	}
	else assert(rv->kind() == OUTPUT_PARAM_KIND);
	return rv;
  }

  const SymTabEntry* lookUp(const char* name) const {
	const SymTabEntry *rv = SymTabEntry::lookUp(name);
	if ((rv == NULL) && (outSt_ != NULL))
	  rv = outSt_->lookUp(name);
	return rv;
  }

  SymTabEntry* lookUp(const char* name) {
	SymTabEntry *rv = SymTabEntry::lookUp(name);
	if ((rv == NULL) && (outSt_ != NULL))
	  rv = outSt_->lookUp(name);
	return rv;
  }

  virtual bool operator==(const Context&) const;
  virtual bool operator!=(const Context& ste) const 
  { return !operator==(ste); };

  virtual void print(ostream& os, int indent=0, bool sort=true) const;

  void buildAuto();

 private:
  SymTab* outSt_;

  unsigned nin_;
  unsigned nout_;
  vector<const char*> inputNm_;
  vector<const char*> outputNm_;

  Session *curSes_;
  vector<Session*> ses_;
};

class Session {
 public:
  Session(Context& par): 
	inval_(), out_() {
	attacksFound_ = false; parent_ = &par;
  };
  Session(Context& par, unsigned n, unsigned m): 
	inval_(), out_() {
	inval_.reserve(n);
	out_.reserve(m);
	attacksFound_ = false; parent_ = &par;
  };

  void insertIn(SymTabEntry &ste, const char *v);
  void insertOut(OutParam &op, SyntaxTree *v);

  unsigned ninputs() const { return inval_.size();};
  unsigned noutputs() const { return out_.size();};
  bool attacksFound() const { return attacksFound_; };
  void attacksFound(bool found) { attacksFound_ = found;};

  const char* inputNm(unsigned i) const {
	return parent_->inputNm(i);
  };
  const char* inputval(unsigned i) const { 
	if (i < inval_.size()) return inval_[i];
    else return NULL;
  };
  char* inputval(unsigned i) { 
	if (i < inval_.size()) return (char *)inval_[i];
    else return NULL;
  };

  const char* outputNm(unsigned i) const {
	return parent_->outputNm(i);
  };
  vector<SyntaxTree*>* outputval(unsigned i) { 
	if (i < out_.size()) return out_[i];
    else return NULL;
  };

 private:
  bool attacksFound_;
  Context* parent_;
  vector<const char*> inval_;
  vector<vector<SyntaxTree*> *> out_; 

 private:
  Session(const Session& s); 
};

#endif
