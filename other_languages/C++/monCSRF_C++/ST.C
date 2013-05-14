#include <algorithm>
#include "ST.h"
#include "STE.h"

#define step " "; 
const int STEP_INDENT = 2;


inline void prtSpace(ostream& os, int n) {
  for (int i = 0; i < n; i++)
    os << step;
}

inline void prtTab(ostream& os, int n) {
  for (int i=0; i < n ; i++)
    os << "\t";
}

inline void prtln(ostream& os, int n) {
  os << endl; prtSpace(os, n);
}

inline void endln(ostream& os, int n) {
  os << ';'; prtln(os, n);
}

SymTab::iterator&
SymTab::iterator::operator++ () {
 if (current != NULL)  
   current = current->next();
 return *this; 
}

SymTab::const_iterator&
SymTab::const_iterator::operator++ () {
  if (current != NULL) 
    current = current->next();
  return *this; 
}

SymTab::SymTab(): map_() {
  first_ = last_ = NULL;
  nelems_ = 0;
}

const SymTabEntry* 
SymTab::lookUp(const char* name) const {
  if (map_.count(name) > 0)
    return map_.find(name)->second; 
  else return NULL;
}

ErrorST 
SymTab::insert(SymTabEntry* se) {
  if (map_.count(se->name()) > 0) {
    //errMsg("Duplicate Symbol " + se->name());
    return DUPLICATE_SYM;
  } 
  else {
    map_[se->name()] = se;
	se->index(nelems_);
	nelems_++;
    se->next(NULL);
    se->prev(last_);
    if (map_.size() == 1)
      first_ = se;
	else last_->next(se);
	last_ = se;
  }

  autoresize(map_);
  return OK;
}

class STE_LT {
 public:
  bool operator()(const SymTabEntry* s1, const SymTabEntry* s2) {
	return ((s1 != NULL) && (s2 != NULL) && 
			((s1->kind() < s2->kind())||
			 (s1->kind() == s2->kind()) && (strcmp(s1->name(),s2->name())<0)));
  }
};

/*
const SymTabEntry* 
SymTab::lookUp(unsigned index) const {
  SymTab::const_iterator it = begin(); 
  for (int i=0; ((it != end()) && (i < index)); i++, it++);
  if (it != end())
	return *it;
  else return NULL;
}
*/

void 
SymTab::printST(ostream& os, int indent, bool sortelems, 
				char leftdelim, char rightdelim, 
				bool linebreak, int first, int last) const {
  int i; SymTab::const_iterator it = begin(); int n_printed=0;
  const SymTabEntry** ste = (const SymTabEntry **)alloca(nelems_ * sizeof(SymTabEntry *));

  //os << "printST(" << indent << ")";
  if ((first == 0) && (last == 0))
	last = 1000000;

  for (i=0; (it != end()) && (i < last); i++, ++it)  {
	if (i >= first) {
	  if (1) { // Replace "1" if some entries are to be suppressed.
		ste[n_printed++] = *it;
	  }
	}
  }

  if (leftdelim != '\0') {
	os << leftdelim;
	if ((n_printed > 0) && (linebreak))
	  prtln(os, indent+STEP_INDENT);
  }

  if (sortelems)
	sort(ste, ste+n_printed, STE_LT());
  // @@@@ Use of sort (instead of stable_sort) leads to a segmentation fault
  // @@@@ This looks like an STL bug but I cant see any thing on Google.

  // @@@@ The above bug disappeared after a bug in the "compare" function
  // @@@@ was fixed. That bug violated the anti-symmetry requirement of
  // @@@@ of the compare operation to be used in sorting. But I would
  // @@@@ expect this violation to perhaps cause an infinite loop, but
  // @@@@ not a segmentation fault.

  for (i=0; i < n_printed; i++) {
	ste[i]->print(os,indent+STEP_INDENT);
	if ((leftdelim == '\0') && (rightdelim != '\0'))
	  os << rightdelim;
	if (i < n_printed-1) {
	  if (linebreak)
		prtln(os,indent+STEP_INDENT);
	  else os << ", ";
	}
	else if (linebreak)
	  prtln(os,indent);
  }

  if (leftdelim != '\0') // This is not a typo -- we shd check leftdelim
	os << rightdelim; 
  //if (linebreak)
  //prtln(os, indent);
}

void SymTab::buildAuto() {
  SymTab::iterator it = begin();
  for (; (it != end()); ++it)
	(*it)->buildAuto();
}

