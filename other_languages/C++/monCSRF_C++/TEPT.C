#include "TEPT.h"

TEPT noTaintedCmdPolicy(NodeTypeSet(CMD_NAME, STMT_SEP), 0.5);
TEPT taintedParamNameSrc(NodeTypeSet(PARAM_NAME), 0.9, "src");
TEPT taintedParamVal(NodeTypeSet(PARAM_VAL), 0.9);
TEPT taintedScriptName(NodeTypeSet(CMD_PARAM), 0, NULL, 0,
					   &taintedParamNameSrc,&taintedParamVal);
TEPT taintedBody(NodeTypeSet(BODY), 0.9);
TEPT taintedScriptNameOrBody(&taintedScriptName, &taintedBody);
TEPT taintedScriptTag(NodeTypeSet(CMD_NAME), 1.0, "script");
TEPT xssPolicy(NodeTypeSet(CMD), 0, NULL, 0, &taintedScriptTag,
			   &taintedScriptNameOrBody);
TEPT csrfPolicy(NodeTypeSet(CMD), 0, NULL, 0, &taintedScriptTag,
			   &taintedScriptNameOrBody);

TEPT updateCmd(NodeTypeSet(CMD_PARAM, CMD_NAME),0,"[uU][pP][dD][aA][tT][eE]");
TEPT tableNameBuyer(NodeTypeSet(CMD_PARAM), 0, "buyer",-1);
TEPT setCmd(NodeTypeSet(CMD_PARAM, CMD_NAME), 0, "[sS][eE][tT]",-1);
TEPT paramEqName(NodeTypeSet(CMD_PARAM), 0, "name");
TEPT operatorEqEq(NodeTypeSet(OPERATOR), 0, NULL, -1);
TEPT taintedParamContSplChar(NodeTypeSet(CMD_PARAM), 0.5,
							 "\"([ -~]*)[^A-z0-9][ -~]*\"", -1);
TEPT problemUpdate(NodeTypeSet(CMD), 0, NULL, 0, &updateCmd, &tableNameBuyer,
			  &setCmd,&paramEqName,&operatorEqEq,&taintedParamContSplChar);

TEPT::
TEPT(NodeTypeSet nodeTypes, float taintThresh, const char *value,
	 short pos, TEPT *child1, TEPT* child2, TEPT *child3,
	 TEPT* child4, TEPT *child5, TEPT* child6): nodeTypes_(nodeTypes) {

  orNode_ = false;
  pos_ = pos;
  if (value != NULL)
	pat_ = new RegExpr(value);
  else pat_ = NULL;
  taintThresh_ = taintThresh;
  matched_ = NULL;
  if (child1 != NULL) child_.push_back(child1);
  if (child2 != NULL) child_.push_back(child2);
  if (child3 != NULL) child_.push_back(child3);
  if (child4 != NULL) child_.push_back(child4);
  if (child5 != NULL) child_.push_back(child5);
  if (child6 != NULL) child_.push_back(child6);
}

TEPT::
TEPT(TEPT *child1, TEPT* child2, TEPT *child3,
	 TEPT* child4, TEPT *child5, TEPT* child6): nodeTypes_() {

  orNode_ = true;
  pos_ = 0;
  pat_ = NULL;
  taintThresh_ = 0;
  matched_ = NULL;
  child_.push_back(child1);
  if (child2 != NULL) child_.push_back(child2);
  if (child3 != NULL) child_.push_back(child3);
  if (child4 != NULL) child_.push_back(child4);
  if (child5 != NULL) child_.push_back(child5);
  if (child6 != NULL) child_.push_back(child6);
}


SyntaxTree* TEPT::
enforce(SyntaxTree *st, int pos) {
  if (nodeEnforce(st, pos))
	return matched_;
  else {
	int j, m = st->numChildren();
	for (j=0; j < m; j++)
	  if (enforce(st->child(j), j+1))
		return matched_;
  }

  return NULL;
}

SyntaxTree* TEPT::
nodeEnforce(SyntaxTree *st, int pos) {
  if (orNode_) {
	int n = child_.size();
	for (int i=0; i < n; i++)
	  if (child_[i]->nodeEnforce(st, pos)) {
		matched_ = st;
		return matched_;
	  };
  }
  else { // Must be an and-node
	if ((pos_ <= 0) || (pos_ == pos)) {
	  if (nodeTypes_.member(st->nodeType())) {
		if (st->taintFrac() >= taintThresh_) {
		  bool m = (pat_ == NULL);
		  if (!m) {
			char c = st->nullTermValue();
			m = pat_->FullMatch(st->value());
			st->undoNullTerm(c);
		  }
		  if (m) {
			int n = child_.size();
			short lastpos = -1;
			for (int i=0; i < n; i++) {
			  short j=child_[i]->pos_-1, l = st->numChildren();
			  if (j == -1) {
				j = lastpos+1;
				for (; j < l; j++) {
				  TEPT* c = child_[i];
				  SyntaxTree* st1 = st->child(j);
				  if (c->nodeEnforce(st1, j+1))
					break;
				}
				if (j >= l)
				  return NULL;
				else lastpos = j;
			  }
			  else {
				if (j < 0) j = lastpos-j-1;
				if (j < l) {
				  TEPT* c = child_[i];
				  SyntaxTree* st1 = st->child(j);
				  if (c->nodeEnforce(st1, j+1))
					lastpos = j;
				  else return NULL;
				}
				else return NULL;
			  }
			}
			matched_ = st;
			return matched_;
		  }
		}
	  }
	}
  }
  return NULL;
}

void TEPT::
printMatched(ostream& os, bool prtNodeFlags) const {
  if (matched_ != NULL)
	matched_->print(os, prtNodeFlags);
  // @@@@ NOT COMPLETED: problem working out the details for orNodes.
}
