#ifndef TEPT_H
#define TEPT_H

#include "SyntaxTree.h"
#include <pcrecpp.h>
typedef pcrecpp::RE RegExpr;
typedef pcrecpp::RE_Options RegExprOpts;

/*
  Taint-Enhanced Policy Trees (TEPTs) have a structure that mirror that of
  SyntaxTrees. The differences are driven by the following factors:

    (a) TEPTs dont need many of the fields of SyntaxTrees
    (b) TEPTs need to specify patterns, so they need disjunctions

  After omitting irrelevant fields from SyntaxTree, we add the features needed
  to support patterns. First, the tree itself is an and-or tree, which
  specifies whether the child subtrees must all match, or just one of them
  needs to match. In the former case, the rest of the fields in the policy node
  are ignored --- it is only the children fields that matter. In the latter
  case the fields of the policy node capture the attributes of a matching
  SyntaxTree node. The attributes that can be specified include:

  -- pos_: the argument position at which this policy node matches.
           Default value of zero matches at any argument position.
  -- nodeTypes_: a bit-wise or of all matching nodeType values.
  -- pat_: a regular expression that needs to match SyntaxTree node's value
  -- taintThresh_: a matching SyntaxTree node must have equal or higher
           fraction of its value tainted.
  -- child_: an array of children TEPTs. For and-TEPT nodes, all child_ TEPTs
           must match the
  -- matched_: after a successful match, this field stores a pointer
           to the SyntaxTree node that matched this policy node.

  // @@@@ May need to incorporate a notion of confidence in taint.

  It is important to note that CYCLES are permitted in TEPTs. While matching
  or-nodes, the possibilities are tried left-to-right, so the recursive case
  must be specified after the non-recursive cases in order for matches to
  be found quickly. Otherwise, the matching algorithm will go on the recursive
  case until it reaches the leaf of a SyntaxTree.

  Examples:
  1. No tainted CMD_NAME or STMT_SEP nodes (for command and SQL injection)

     pos_ = 0
     nodeTypes_ = CMD_NAME | STMT_SEP
     taintThresh_ = 0.5
	 child_ = pat_ = NULL

	 To simplify textual representation, we use:
         InjectionPolicy ::= pat_{NodeTypes}^{taintThresh}(children),
     where a square bracket replaces parenthesis in the case of and-nodes.
     Braces are optional, as are the node types and taint threshold.
     (An omitted node type implies a match for all types of nodes; an
     omitted threshold is assumed to be equal to zero.) To ease understanding,
     components of a policy may be named. The above TEPT is represented
     as follows:

         "*"_{CMD_NAME|STMT_SEP}^{0.5}

  2. No tainted script tags with script name or body that is tainted (XSS)

       TaintedScriptName ::=
          "*"_{CMD_PARAM}["src"_{PARAM_NAME}^{1.0}, "*"_{PARAM_VALUE}^{0.9}]
       TaintedScriptBody ::=
          "*"_{CMD_PARAM}["*"_{BODY}^{0.9}]

       XSS_Policy ::= "*"_{CMD}["script"_{CMD_NAME}^{1.0},
	                            (TaintedScriptName, TaintedScriptBody)]

  3. No nested tables in HTML (just an example)

       TableTag ::= "table"_{CMD_NAME}
	   NonTableNodeWithNestedTable ::=
	      "*"_{~CMD_NAME}[(TableTag, NonTableNodeWithNestedTable)]
	   NestedTables ::= "*"_{CMD}[TableTag, NonTableNodeWithNestedTable]

  Semantics of matching:
  (a) A match of a TEPT is attempted at every subtree of a SyntaxTree. To
      limit to root-matches, include nodeType == ROOT in the TEPT.
  (b) There is a node-level match at a SyntaxTree node st if the node contents
      match in terms of the pos_, value, taint, and nodeType.
  (c) There is a complete match at st if there is a node-level match, and
       (i) for and-nodes, for each child_ of this TEPT, there is a child of
           st that matches it. Note, however, that multiple children of TEPT
		   may match the same child of st. On the same note, it is possible
           (and typical) that there aren't any TEPT subtrees that match some
           of the children of st.
      (ii) for or-nodes: These can appear only as the children of and-nodes.
	       In this case, a SyntaxTree st matches a TEPT if any of the
		   child_ of TEPT matches st.

*/

class NodeTypeSet {
 private:
  unsigned int nts_;

 public:
  NodeTypeSet() { nts_ = 0;}
  NodeTypeSet(NodeType n1) { nts_ = (1 << n1);}
  NodeTypeSet(NodeType n1, NodeType n2) { nts_ = (1<<n1)|(1<<n2);}
  NodeTypeSet(NodeType n1, NodeType n2, NodeType n3) {
	nts_ = (1<<n1)|(1<<n2)|(1<<n3);}
  NodeTypeSet(NodeType n1, NodeType n2, NodeType n3, NodeType n4) {
	nts_ = (1<<n1)|(1<<n2)|(1<<n3)|(1<<n4);}
  NodeTypeSet(NodeType n1, NodeType n2, NodeType n3, NodeType n4, NodeType n5){
	nts_ = (1<<n1)|(1<<n2)|(1<<n3)|(1<<n4)|(1<<n5);}

  bool member(NodeType nt) const { return ((nts_ & (1 << nt)) != 0); };
  void ntunion(NodeTypeSet n1) { nts_ |= n1.nts_; }
  void ntunion(NodeType nt) { nts_ |= (1<<nt); }
  void ntintersection(NodeTypeSet n1) { nts_ &= n1.nts_; }
  void complement() { nts_ = ~nts_; }
};

class TEPT {
 public:
  TEPT(NodeTypeSet nodeTypes, float taintThresh=0.7, const char *value=NULL,
	   short pos=0,
	   TEPT *child1 = NULL, TEPT* child2 = NULL, TEPT *child3 = NULL,
	   TEPT* child4 = NULL, TEPT *child5 = NULL, TEPT* child6 = NULL);
  TEPT(TEPT *child1, TEPT* child2 = NULL, TEPT *child3 = NULL, //or-node
	   TEPT* child4 = NULL, TEPT *child5 = NULL, TEPT* child6 = NULL);


  unsigned short pos() const { return pos_; };
  void pos(short p) { pos_ = p; };
  void replacePos(short opos, short npos) {
	if (pos_ == opos) pos_ = npos;
	for (int i=child_.size()-1; i >=0; i--)
	  child_[i]->replacePos(opos, npos);
  };

  SyntaxTree* matched() { return matched_;};
  void matched(SyntaxTree* st) { matched_ = st;};
  void matched(SyntaxTree* st, SyntaxTree* st1) {
	matched_ = st; if (child_.size() > 0) child_[0]->matched(st1);};
  void matched(SyntaxTree* st, SyntaxTree* st1, SyntaxTree* st2) {
	matched_ = st;
	if (child_.size() > 0) child_[0]->matched(st1);
	if (child_.size() > 1) child_[1]->matched(st2);};
  void clearMatched() { matched_ = NULL;};

  void printMatched(ostream& os=cout, bool prtNodeFlags=false) const;
  SyntaxTree* enforce(SyntaxTree *s, int pos=0);
  SyntaxTree* nodeEnforce(SyntaxTree*, int pos);
  SyntaxTree* nodeEnforce(SyntaxTree* st) {
	assert((!orNode_) && (pos_ == 0)); // Not implemented yet for other cases
	if (st != NULL)
	  return nodeEnforce(st, 0);
	else return NULL;
  };

 private:
  bool orNode_;         // Is this an "and"-node or an "or"-node?
  short pos_;           // matches at this argument position (0 = any position)
  NodeTypeSet nodeTypes_;// A bit mask identifying possible node types
  RegExpr *pat_;        // regular expression on node value
  float taintThresh_;   // at least taintThresh shd be tainted.
  SyntaxTree* matched_; // Stores a pointer to the matching SyntaxTree node
  vector<TEPT*> child_;//
};

extern TEPT noTaintedCmdPolicy;
extern TEPT xssPolicy;
extern TEPT csrfPolicy;
extern TEPT problemUpdate;
#endif
