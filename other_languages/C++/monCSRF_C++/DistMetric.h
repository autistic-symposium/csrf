#ifndef DIST_METRIC_H
#define DIST_METRIC_H

#define DEFAULT_COST 20
class DistMetric {
 public:
  DistMetric();
  void init();

  int insCost(char c) const { return inscost[c]; }
  int delCost(char c) const { return delcost[c]; }
  int subCost(char c1, char c2) const 
	{ return subcost[c1][c2]; }
  char normalize(char c) const { return normal[c]; }

  virtual int geticost(char sc) const =0;
  virtual int getdcost(char pc) const =0;
  virtual int getscost(char pc, char sc) const =0;
  virtual char getnorm(char c) const =0;
  
 protected:
  char cc[256];
  char normal[256];
  //int cc_insCost[256];
  //int cc_delCost[256];
  //int cc_subCost[256][256];

 private:
  unsigned char inscost[256];
  unsigned char delcost[256];
  unsigned char subcost[256][256];
};

class TextMetric: public DistMetric {
 public:
  static const char OPTIONAL_CHAR = '\2';
  static const char WILD_CARD_CHAR = '\1';
  static const char CC_BINARY = 0;
  static const char CC_CTRL = 1;
  static const char CC_SPACE = 2;
  static const char CC_PUNCT = 3;
  static const char CC_QUOTE = 4;
  static const char CC_SPL = 5;
  static const char CC_DEC = 6;
  static const char CC_ALPHA = 7;
  static const char CC_ESC = 8;

 public:
  TextMetric();
  int geticost(char sc) const;
  int getdcost(char pc) const;
  int getscost(char pc, char sc) const;
  char getnorm(char c) const;
};

class BinMetric: public DistMetric {
 public:
  BinMetric();
  int geticost(char sc) const;
  int getdcost(char pc) const;
  int getscost(char pc, char sc) const;
  char getnorm(char c) const;
};
#endif
