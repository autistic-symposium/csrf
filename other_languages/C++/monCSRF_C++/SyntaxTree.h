#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <vector>
#include "Compatibility.h"

#include "p1.h"
#include "ParserUtil.h"

class TEPT;

enum NodeType { // @@@@ NOTE: must be < 64 types
  ROOT_NODE,
  STMT_SEP, OPERATOR,
  CMD, CMD_NAME, CMD_PARAM, CMD_REFFER,
  PARAM_NAME, PARAM_VAL,
  GRP, GRP_BEGIN, GRP_END,
  BODY,
  INPUT_NODE,  OUTPUT_NODE,
  MULTIPLE,
  ERROR_NT
};

const char* getNodeName(NodeType nt);

extern char canonical[];

struct TaintInfo {
  unsigned len_;
  const char *begin_;
  char* edits_;
  const char *src_;
  const char *srcNm_;
  int srcOffset_;

  TaintInfo() {
	len_ = 0;
  }
};

struct ParamInfo {
  const char *paramName_;
  char *constPrefix_;
  char *inputEdits_;
  char *constSuffix_;
};

class SyntaxTree;
bool eqSTree(const SyntaxTree* st1, const SyntaxTree* st2);
bool lessSTree(const SyntaxTree* st1, const SyntaxTree* st2);
size_t hashSTree(const SyntaxTree* st);
HashCmpClass(STree, const SyntaxTree*, hashSTree, eqSTree, lessSTree);
typedef HashMap(STree, const SyntaxTree*, SyntaxTree*) SyntaxTreeTab;

class SyntaxTree {
 public:
  SyntaxTree(NodeType nodeType, char *beg=NULL, int len=0, u_char conf=255);
  SyntaxTree(NodeType nodeType, char *beg, int len,
			 vector<SyntaxTree*>* child, u_char conf=255);
  SyntaxTree(NodeType nodeType, char *beg, int len, u_char conf,
			 SyntaxTree *child1, SyntaxTree* child2 = NULL);
  SyntaxTree(NodeType nodeType, SyntaxTree *child1, SyntaxTree* child2 = NULL);

  bool isParameterized() const { return (bool)(flags_ & 0x40);};
  void isParameterized(bool p) { flags_ = p ? (flags_|0x40):(flags_&0xbf);};
  bool isNodeTainted() const {
	return ((!isParameterized()) &&
			(taintInfo_ != NULL) && (taintInfo_->len_ > 0));}
  bool isSubTreeTainted() const {
	return (isNodeTainted() || (flags_ & 0x80));}
  void isSubTreeTainted(bool p) {
	flags_ = p ? (flags_ | 0x80) : (flags_ & 0x7f);};
  NodeType nodeType() const { return (NodeType)(flags_ & 0x3f);};
  void nodeType(NodeType nt) { flags_ = ((flags_ & 0xc0) | nt);};
  u_char confidence() const { return confidence_; }
  void confidence(u_char c) { confidence_ = c; };
  u_short vLen() const { return vlen_;}
  const char *value() const { return val_;}
  void value(char *begin, u_short len) { val_ = begin; vlen_ = len; };

  int numChildren() const { return (child_ == NULL ? 0 : child_->size()); }
  const SyntaxTree *child(unsigned i) const {
	if ((child_ != NULL) && (i < child_->size())) return (*child_)[i];
	else return NULL;
  }
  SyntaxTree *child(unsigned i) {
	if ((child_ != NULL) && (i < child_->size())) return (*child_)[i];
	else return NULL;
  }
  void child(unsigned i, SyntaxTree *st) {
	assert ((child_ != NULL) && (i < child_->size())); (*child_)[i] = st;
  }
  void addChild(SyntaxTree *s) {
	if (child_ == NULL) child_ = new vector<SyntaxTree*>();
	child_->push_back(s);
  }
  void insert(int pos, SyntaxTree *s) {
	vector<SyntaxTree*>::iterator si;
	if (child_ == NULL) child_ = new vector<SyntaxTree*>();
	si = child_->begin();
	for (int i=0;
		 (i < pos) && (si != child_->end()); i++, si++);
	child_->insert(si, s);
  }
  void append(vector<SyntaxTree*>& st) {
	if (child_ == NULL) child_ = new vector<SyntaxTree*>();
	child_->insert(child_->end(), st.begin(), st.end());
  }
  void append(SyntaxTree *st) {
	if ((st != NULL) && (st->child_ != NULL))
	  append(*st->child_);
  }

  float taintFrac() const {
	float rv;
	if (isParameterized())
	  return 0.0;
	else if ((taintInfo_ != NULL) && (taintInfo_->len_ > 0))
	  if (vlen_ != 0)
		rv = (((float)(taintInfo_->len_))/vlen_);
	  else rv= 1.0;
	else rv= 0.0;
	//cout << "\nTF: " << rv;
	return rv;
  }

  int  cmpNodeVal(const SyntaxTree* st) const;
  bool eqNodeVal(const SyntaxTree* st) const { return (cmpNodeVal(st) == 0);};
  bool lessNodeVal(const SyntaxTree* st) const {return (cmpNodeVal(st)==-1);};

  int  shallowCmpTreeVal(const SyntaxTree* st) const;
  bool shallowEqTreeVal(const SyntaxTree* st) const
	{ return (shallowCmpTreeVal(st) == 0); };
  bool shallowLessTreeVal(const SyntaxTree* st) const
	{ return (shallowCmpTreeVal(st) == -1); };

  size_t hashNodeVal() const;
  size_t shallowHashTreeVal() const;

  SyntaxTree* replaceChildren(SyntaxTree* st1, SyntaxTree* st2=NULL) const;
  SyntaxTree *shallowCopy() const;
  SyntaxTree *deepCopy() const;

  SyntaxTree* dagify(SyntaxTreeTab& t);

  void print(ostream& os=cout, int prtNodeFlags=0, int depthThresh=100000,
			 u_char confThresh=50, unsigned indnt=2,
			 bool preserveWS=false) const; // omit values with lower conf
  void printall(ostream& os=cout, int prtNodeFlags=0) const {
	print(os, prtNodeFlags, 100000, 0);}

  SyntaxTree *abstract() const;

  void merge(const SyntaxTree *st);
  void sortUniqueChildren();

  char nullTermValue() {
	char rv=0;
	if (val_ != NULL) {
	  rv = val_[vlen_];
	  val_[vlen_] = '\0';
	}
	return rv;
  }
  void undoNullTerm(char c) {
	if (val_ != NULL)
	  val_[vlen_] = c;
  }

  /* @@@@ Following functions dont handle isParameterized_ @@@@ */

  bool initTaint(const char *s, const char *pnm, const char *p, int matchbeg,
				 int matchend, char *opmap, TEPT* crossNodesPolicy,
				 TEPT* straddleTreesPolicy, bool *ltm=NULL, bool* rtm=NULL);
  // assumes that begin_ points within s

  SyntaxTree* maxCommPrefix(const SyntaxTree& st, u_char confThresh=50) const;
  int match(const SyntaxTree& st, u_char confThresh=50);

  /* @@@@ Above functions dont handle isParameterized_ @@@@ */

 private:
  void initFields(NodeType nodeType, char *beg, int len, u_char s,
				  vector<SyntaxTree*> *child=NULL);
  SyntaxTree(const SyntaxTree&);

 private:
  u_char flags_;
  u_char confidence_;
  u_short vlen_;
  char* val_;
  union {
	TaintInfo* taintInfo_;
	ParamInfo* paramInfo_;
  };
  vector<SyntaxTree*>* child_;
};

inline ostream& operator<<(ostream& os, const SyntaxTree* st) {
  st->print(os); return os;
};

#endif
