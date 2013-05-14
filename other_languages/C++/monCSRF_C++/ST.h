#ifndef ST_H
#define ST_H

#include <cstring>
#include "Compatibility.h"

class SymTabEntry;

inline bool lessString(const char* s1, const char* s2) {
  return (strcmp(s1, s2) < 0);
};
inline bool eqString(const char* s1, const char* s2) {
  return (strcmp(s1, s2) == 0);
};

inline size_t hashString(const char* s) {
  size_t rv = 0;
  while (*s)
	rv = ((rv << 5) + rv) ^ (*s++);
  return rv;
}
inline size_t hashString(const char* s, int len) {
  size_t rv = 0;
  for (int i = 0; i < len; i++)
	rv = ((rv << 5) + rv) ^ (s[i]);
  return rv;
}
HashCmpClass(String, const char*, hashString, eqString, lessString);

enum ErrorST {OK, DUPLICATE_SYM, INVALID_SCOPE, SYM_NOT_PRESENT};

class SymTab {
 public:
  class const_iterator {
   private:
    const SymTabEntry* current;
   public:
    const_iterator(const SymTabEntry *c) { current = c; };
    const SymTabEntry* operator* () const { return current;};
    const_iterator & operator++ ();
    bool operator==(const const_iterator& i) const
	  {return (current == i.current);};
    bool operator!=(const const_iterator& i) const
      {return !(operator==(i));};
  };

  class iterator {
   private:
    SymTabEntry* current;
   public:
    iterator(SymTabEntry *c) { current = c; };
    SymTabEntry* operator* () { return current;};
    iterator & operator++ ();
    bool operator==(const iterator& i) const {return (current == i.current);};
    bool operator!=(const iterator& i) const {return !operator==(i);};
  };

 public:
  SymTab();
  virtual ~SymTab() {};

  int size() const {return map_.size();};
  int nelems() const {return nelems_;};

  const SymTabEntry* lookUp(const char* name) const;
  SymTabEntry* lookUp(const char* name) {
    return (SymTabEntry*)(((const SymTab*)this)->lookUp(name));
  }

  ErrorST insert(SymTabEntry*); // will resize automatically
  void print(ostream& os,int indent=0, bool sort=true) const
	{printST(os, indent, sort);};
  virtual void printST(ostream& os,int ind=0,bool sort=true,
					   char ldelim='{',char rdelim='}',
					   bool linebreaks=true, int first=0, int last=0) const;

  void buildAuto();

  const_iterator begin() const { return const_iterator(first_); };
  iterator begin() { return iterator(first_); };
  // End of list is indicated when the iterator's current pointer
  // equals NULL, but not when it is equal to last_.
  const_iterator end() const { return const_iterator(NULL); };
  iterator end() { return iterator(NULL); };

 private:

  HashMap(String, const char*, SymTabEntry *) map_;
  SymTabEntry* first_;
  SymTabEntry* last_;
  int nelems_;

 private: // Disable copy and assignment.
  SymTab(const SymTab&);
  SymTab& operator=(const SymTab &);
};

#endif

