#ifndef TREE_AUTO_H
#define TREE_AUTO_H

using namespace std;

#include <vector>
#include <list>
#include <map>
#include <iostream>
#include "STE.h"
#include "SyntaxTree.h"
#include "p1.h"
#include <assert.h>

class TreeAuto;

#define POS_NOT_PRESENT -1
#define SYM_MISMATCH -2
#define UNEXPECTED_TAINT -3
#define SUBTREE_MISMATCH -4
#define UNEXPECTED_SUBTREE -5

class Position {
 private:
  static const unsigned FLAG_BITS = 1;
  static const unsigned PAR_BITS = 13; // **** SHD BE 18
  static const unsigned CHI_BITS = 13;
 public:
  static const unsigned MAX_PARENT = ((1 << PAR_BITS)-2);
  static const unsigned MAX_CHILD  = ((1 << CHI_BITS)-2);
 private:
  static const unsigned CHI_MASK = (1<<CHI_BITS)-1;
  static const unsigned CHI_SHIFT_BITS = FLAG_BITS;
  static const unsigned PAR_SHIFT_BITS = (CHI_SHIFT_BITS+CHI_BITS);

  static unsigned mkdata(unsigned par, unsigned chi) {
	assert((par <= MAX_PARENT+1) && (chi <= MAX_CHILD));
	return ((par << PAR_SHIFT_BITS) | (chi << CHI_SHIFT_BITS) | 0x1);
  };
  static unsigned mkdata(const char *nm) {
	assert (((unsigned)nm & 0x1)==0); // ptr to be aligned on 2-byte boundary
	return (unsigned int)nm;
  };
  static unsigned getpar(unsigned d) {
	assert (d & 0x1 == 1);
	return (d >> PAR_SHIFT_BITS);
  };
  static unsigned getch(unsigned d) {
	assert (d & 0x1 == 1);
	return ((d >> CHI_SHIFT_BITS) & CHI_MASK);
  };
  static const char *getnm(unsigned d) {
	assert (d & 0x1 == 0);
	return (const char *)d;
  };

 public:
  Position() { data_ = 0;};
  Position(unsigned parent, unsigned child) {data_ = mkdata(parent, child);}
  Position(char *childName) {data_ = mkdata(childName);};

  unsigned parent() const {return getpar(data_);};
  unsigned child() const {return getch(data_);};
  const char* name() const {return getnm(data_);};

  void parent(unsigned p) {data_ = mkdata(p, getch(data_));};
  void child(unsigned c) {data_ = mkdata(getpar(data_), c);};
  void name(const char* nm) {data_ = mkdata(nm);};

  size_t hash() const { return data_;};
  bool operator==(const Position& p) const {return (data_==p.data_);};
  bool operator!=(const Position& p) const {return !operator==(p);};
  bool occursBefore(const Position &p) const {return (data_ < p.data_);}

  void print(ostream& os=cout) const {
	if (data_ & 0x1)
	  os << '(' << parent() << ',' << child() << ')';
	else os << name();
  }

 private:
  unsigned int data_;
};

inline ostream& operator<<(ostream& os, const Position& p) { 
  p.print(os); return os;
};

extern const Position null_pos;

class OccursBefore {
 public:
  bool operator()(const Position &p1, const Position& p2) const
	{ return p1.occursBefore(p2);};
};

struct LeafElem {
  SyntaxTree *st_;
  int num_;
 public:
  LeafElem(SyntaxTree* st=NULL) {st_ = st; num_ = 1;};
};

typedef map<Position, SyntaxTree*, OccursBefore> Fringe;
typedef map<Position, LeafElem, OccursBefore> LeafInfo;
typedef slist<Fringe*> FringeList;

inline bool lessSTNV(const SyntaxTree* s1, const SyntaxTree* s2) {
  return (s1->lessNodeVal(s2));
};

inline bool eqSTNV(const SyntaxTree* s1, const SyntaxTree* s2) {
  return (s1->eqNodeVal(s2));
};

inline size_t hashSTNV(const SyntaxTree* s) {
  return (s->hashNodeVal());
};
HashCmpClass(STNV, SyntaxTree*, hashSTNV, eqSTNV, lessSTNV);
typedef HashMap(STNV, SyntaxTree*, TreeAuto*) Transition;
typedef HashSet(STNV, SyntaxTree*) STNVSet;

inline size_t hashSTP(SyntaxTree *p) { return (size_t)((unsigned)p); };
inline bool lessSTP(SyntaxTree *v1,  SyntaxTree* v2) { return (v1 < v2);};
inline bool eqSTP(SyntaxTree *v1,  SyntaxTree* v2) { return (v1 == v2);};
HashCmpClass(STP,  SyntaxTree*, hashSTP, eqSTP, lessSTP);
typedef HashSet(STP, SyntaxTree*) STSet;


class TreeAuto {
 public:
  TreeAuto(PatSet& ps);

  void print(ostream& os=cout, int indent=0, const Position& prev=null_pos) const;

 public:
  int enforce(SyntaxTree& st) const;

 private:
  TreeAuto(int d, PatSet& ps, FringeList& unseen, bool fastButSloppy);
  Position select(FringeList& unseen, bool fastButSloppy);
  void buildAuto(int d, PatSet& ps, FringeList& rs, bool fastButSloppy);
  int enforce(vector<SyntaxTree*>& r, int remainingNodes) const;

 private:
  int num_; // Number of inputs reaching this state
  Position pos_;
  union {
	Transition *trans_; // Present for internal nodes
	LeafInfo *leaf_; // Present for leaf nodes
  };
};

inline ostream& operator<<(ostream& os, const TreeAuto& ta) { 
  ta.print(os); return os;
};

#endif
