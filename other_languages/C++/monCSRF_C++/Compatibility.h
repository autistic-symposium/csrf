#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#ifdef _OSX
typedef unsigned char u_char;
typedef unsigned short u_short;
#endif

#ifdef _WIN
#define YY_NO_UNISTD_H
#include <windows.h>
#include <ostream>
#include <hash_map>
#include <hash_set>
#include <functional>
#include <list>
#define slist list
using namespace std;
using namespace stdext;

#define round(x) (int)((x)+0.5)
#define FOPEN_READ_MODE "rb"
#define strcasecmp _stricmp
#define strncasecmp  _strnicmp

#define HashCmpClass(baseName, className, hashFn, eqFn, lessFn) \
   class WinHash ## baseName : public stdext::hash_compare< className > { \
      public: \
        size_t operator()(/*const*/ className p) const { \
            return hashFn(p); } \
        bool operator()(/*const*/className p1, /*const*/className p2) const { \
            return lessFn(p1, p2); } \
   }

#define HashMap(baseName, keyClass, valueClass) \
   hash_map< keyClass, valueClass, WinHash ## baseName >

#define HashSet(baseName, keyClass) \
   hash_set< keyClass, WinHash ## baseName >

#define autoresize(x)

#else
#include <ext/hash_map>
#include <ext/hash_set>
//#include <ext/hash_fun.h>
#include <backward/hash_fun.h>
#include <ext/slist>
#include <math.h>
#define slist __gnu_cxx::slist

#define FOPEN_READ_MODE "r"

#define HashCmpClass(baseName, className, hashFn, eqFn, lessFn) \
   class Hash ## baseName { \
      public: \
        size_t operator()(/*const*/ className p) const { \
            return hashFn(p); } \
   }; \
   class Eq ## baseName { \
      public: \
        bool operator()(/*const*/className p1, /*const*/className p2) const { \
            return eqFn(p1, p2); } \
   }

#define HashMap(baseName, keyClass, valueClass) \
  __gnu_cxx::hash_map< keyClass, valueClass, Hash ## baseName, Eq ## baseName >

#define HashSet(baseName, keyClass) \
  __gnu_cxx::hash_set< keyClass, Hash ## baseName, Eq ## baseName >

#define autoresize(x) \
    if ((x).bucket_count() <= (x).size()) (x).resize(2*(x).bucket_count())

using namespace std;
#define USHORT unsigned short
#define ULONG  unsigned long
#define BYTE   unsigned char

#endif

#endif
