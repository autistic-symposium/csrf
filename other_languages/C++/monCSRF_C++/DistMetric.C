#include <ctype.h>
#include "DistMetric.h"

#define scale(x, y)   (int)((x*(double)y) + 0.5)

DistMetric::DistMetric() {
  for (int i=0; i < 256; i++) {
	cc[i] = 0;
	normal[i] = i;
	inscost[i] = 0;
	delcost[i] = 0;
	for (int j=0; j < 256; j++)
	  subcost[i][j] = 0;
/*
	cc_insCost[i] = DEFAULT_COST;
	cc_delCost[i] = DEFAULT_COST;
	for (int j=0; j < 256; j++)
	  if (i == j)
		cc_subCost[i][j] = 0;
	  else cc_subCost[i][j] = DEFAULT_COST;
*/
  }
}

void
DistMetric::init() {
  for (int i=0; i < 256; i++) {
/*
	if (insCost[i] == DEFAULT_COST) // Not initialized yet
	  insCost[i] = cc_insCost[cc[i]];
	if (delCost[i] == DEFAULT_COST) // Not initialized yet
	  delCost[i] = cc_delCost[cc[i]];
*/

	if (inscost[i] == 0)
	  inscost[i] = geticost(i);
	if (delcost[i] == 0)
	  delcost[i] = getdcost(i);
	if (normal[i] == i)
	  normal[i] = getnorm(i);
	for (int j=0; j < 256; j++) {
/*	  if (subCost[i][j] == DEFAULT_COST)
		subCost[i][j] = cc_subCost[cc[i]][cc[j]];
*/
	  if (subcost[i][j] == 0)
		subcost[i][j] = getscost(i, j);
	}
  }
}

TextMetric::TextMetric(): DistMetric() {
  for (int i=0; i < 255; i++)
	cc[i] = CC_BINARY;

  cc[0]   = CC_CTRL ; /* NUL */
  cc[1]   = CC_CTRL ; /* SOH */
  cc[2]   = CC_CTRL ; /* STX */
  cc[3]   = CC_CTRL ; /* ETX */
  cc[4]   = CC_CTRL ; /* EOT */
  cc[5]   = CC_CTRL ; /* ENQ */
  cc[6]   = CC_CTRL ; /* ACK */
  cc[7]   = CC_CTRL ; /* BEL */
  cc[8]   = CC_CTRL ; /* BS */
  cc[9]   = CC_SPACE; /* HT */
  cc[10]  = CC_SPACE; /* LF */
  cc[11]  = CC_SPACE; /* VT */
  cc[12]  = CC_SPACE; /* FF */
  cc[13]  = CC_SPACE; /* CR */
  cc[14]  = CC_CTRL ; /* SO */
  cc[15]  = CC_CTRL ; /* SI */
  cc[16]  = CC_CTRL ; /* DLE */
  cc[17]  = CC_CTRL ; /* DC1 */
  cc[18]  = CC_CTRL ; /* DC2 */
  cc[19]  = CC_CTRL ; /* DC3 */
  cc[20]  = CC_CTRL ; /* DC4 */
  cc[21]  = CC_CTRL ; /* NAK */
  cc[22]  = CC_CTRL ; /* SYN */
  cc[23]  = CC_CTRL ; /* ETB */
  cc[24]  = CC_CTRL ; /* CAN */
  cc[25]  = CC_CTRL ; /* EM */
  cc[26]  = CC_CTRL ; /* SUB */
  cc[27]  = CC_CTRL ; /* ESC */
  cc[28]  = CC_CTRL ; /* FS */
  cc[29]  = CC_CTRL ; /* GS */
  cc[30]  = CC_CTRL ; /* RS */
  cc[31]  = CC_CTRL ; /* US */
  cc[32]  = CC_SPACE; /* SPC */
  cc[33]  = CC_PUNCT; /* ! */
  cc[34]  = CC_QUOTE; /* " */
  cc[35]  = CC_SPL;   /* # */
  cc[36]  = CC_SPL;   /* $ */
  cc[37]  = CC_SPL;   /* % */
  cc[38]  = CC_SPL;   /* & */
  cc[39]  = CC_QUOTE; /* ' */
  cc[40]  = CC_SPL;   /* ( */
  cc[41]  = CC_SPL;   /* ) */
  cc[42]  = CC_SPL;   /* * */
  cc[43]  = CC_SPL;   /* + */
  cc[44]  = CC_PUNCT; /* , */
  cc[45]  = CC_SPL;   /* - */
  cc[46]  = CC_PUNCT; /* . */
  cc[47]  = CC_SPL;   /* / */
  cc[48]  = CC_DEC;   /* 0 */
  cc[49]  = CC_DEC;   /* 1 */
  cc[50]  = CC_DEC;   /* 2 */
  cc[51]  = CC_DEC;   /* 3 */
  cc[52]  = CC_DEC;   /* 4 */
  cc[53]  = CC_DEC;   /* 5 */
  cc[54]  = CC_DEC;   /* 6 */
  cc[55]  = CC_DEC;   /* 7 */
  cc[56]  = CC_DEC;   /* 8 */
  cc[57]  = CC_DEC;   /* 9 */
  cc[58]  = CC_PUNCT; /* : */
  cc[59]  = CC_PUNCT; /* ; */
  cc[60]  = CC_SPL;   /* < */
  cc[61]  = CC_SPL;   /* = */
  cc[62]  = CC_SPL;   /* > */
  cc[63]  = CC_PUNCT; /* ? */
  cc[64]  = CC_SPL;   /* @ */
  cc[65]  = CC_ALPHA; /* A */
  cc[66]  = CC_ALPHA; /* B */
  cc[67]  = CC_ALPHA; /* C */
  cc[68]  = CC_ALPHA; /* D */
  cc[69]  = CC_ALPHA; /* E */
  cc[70]  = CC_ALPHA; /* F */
  cc[71]  = CC_ALPHA; /* G */
  cc[72]  = CC_ALPHA; /* H */
  cc[73]  = CC_ALPHA; /* I */
  cc[74]  = CC_ALPHA; /* J */
  cc[75]  = CC_ALPHA; /* K */
  cc[76]  = CC_ALPHA; /* L */
  cc[77]  = CC_ALPHA; /* M */
  cc[78]  = CC_ALPHA; /* N */
  cc[79]  = CC_ALPHA; /* O */
  cc[80]  = CC_ALPHA; /* P */
  cc[81]  = CC_ALPHA; /* Q */
  cc[82]  = CC_ALPHA; /* R */
  cc[83]  = CC_ALPHA; /* S */
  cc[84]  = CC_ALPHA; /* T */
  cc[85]  = CC_ALPHA; /* U */
  cc[86]  = CC_ALPHA; /* V */
  cc[87]  = CC_ALPHA; /* W */
  cc[88]  = CC_ALPHA; /* X */
  cc[89]  = CC_ALPHA; /* Y */
  cc[90]  = CC_ALPHA; /* Z */
  cc[91]  = CC_SPL;   /* [ */
  cc[92]  = CC_ESC;   /* \ */
  cc[93]  = CC_SPL;   /* ] */
  cc[94]  = CC_SPL;   /* ^ */
  cc[95]  = CC_SPL;   /* _ */
  cc[96]  = CC_QUOTE; /* ` */
  cc[97]  = CC_ALPHA; /* a */
  cc[98]  = CC_ALPHA; /* b */
  cc[99]  = CC_ALPHA; /* c */
  cc[100] = CC_ALPHA; /* d */
  cc[101] = CC_ALPHA; /* e */
  cc[102] = CC_ALPHA; /* f */
  cc[103] = CC_ALPHA; /* g */
  cc[104] = CC_ALPHA; /* h */
  cc[105] = CC_ALPHA; /* i */
  cc[106] = CC_ALPHA; /* j */
  cc[107] = CC_ALPHA; /* k */
  cc[108] = CC_ALPHA; /* l */
  cc[109] = CC_ALPHA; /* m */
  cc[110] = CC_ALPHA; /* n */
  cc[111] = CC_ALPHA; /* o */
  cc[112] = CC_ALPHA; /* p */
  cc[113] = CC_ALPHA; /* q */
  cc[114] = CC_ALPHA; /* r */
  cc[115] = CC_ALPHA; /* s */
  cc[116] = CC_ALPHA; /* t */
  cc[117] = CC_ALPHA; /* u */
  cc[118] = CC_ALPHA; /* v */
  cc[119] = CC_ALPHA; /* w */
  cc[120] = CC_ALPHA; /* x */
  cc[121] = CC_ALPHA; /* y */
  cc[122] = CC_ALPHA; /* z */
  cc[123] = CC_SPL;   /* { */
  cc[124] = CC_SPL;   /* | */
  cc[125] = CC_SPL;   /* } */
  cc[126] = CC_SPL;   /* ~ */
  cc[127] = CC_CTRL;  /* DEL */

  //normal[0]   = CC_CTRL ; /* NUL */
  //normal[1]   = CC_CTRL ; /* SOH */
  //normal[2]   = CC_CTRL ; /* STX */
  //normal[3]   = CC_CTRL ; /* ETX */
  //normal[4]   = CC_CTRL ; /* EOT */
  //normal[5]   = CC_CTRL ; /* ENQ */
  //normal[6]   = CC_CTRL ; /* ACK */
  normal[7]   = OPTIONAL_CHAR ; /* BEL */
  normal[8]   = OPTIONAL_CHAR ; /* BS */
  normal[9]   = OPTIONAL_CHAR; /* HT */
  normal[10]  = OPTIONAL_CHAR; /* LF */
  normal[11]  = OPTIONAL_CHAR; /* VT */
  normal[12]  = OPTIONAL_CHAR; /* FF */
  normal[13]  = OPTIONAL_CHAR; /* CR */
  normal[14]  = OPTIONAL_CHAR ; /* SO */
  normal[15]  = OPTIONAL_CHAR ; /* SI */
  normal[16]  = OPTIONAL_CHAR ; /* DLE */
  normal[17]  = OPTIONAL_CHAR ; /* DC1 */
  normal[18]  = OPTIONAL_CHAR ; /* DC2 */
  normal[19]  = OPTIONAL_CHAR ; /* DC3 */
  normal[20]  = OPTIONAL_CHAR ; /* DC4 */
  normal[21]  = OPTIONAL_CHAR ; /* NAK */
  normal[22]  = OPTIONAL_CHAR ; /* SYN */
  normal[23]  = OPTIONAL_CHAR ; /* ETB */
  normal[24]  = OPTIONAL_CHAR ; /* CAN */
  normal[25]  = OPTIONAL_CHAR ; /* EM */
  normal[26]  = OPTIONAL_CHAR ; /* SUB */
  normal[27]  = OPTIONAL_CHAR ; /* ESC */
  normal[28]  = OPTIONAL_CHAR ; /* FS */
  normal[29]  = OPTIONAL_CHAR ; /* GS */
  normal[30]  = OPTIONAL_CHAR ; /* RS */
  normal[31]  = OPTIONAL_CHAR ; /* US */
  normal[32]  = '_'; /* SPC */
  //normal[33]  = CC_PUNCT; /* ! */
  normal[34]  = OPTIONAL_CHAR; /* " */
  normal[35]  = '_';   /* # */
  //normal[36]  = CC_SPL;   /* $ */
  //normal[37]  = CC_SPL;   /* % */
  //normal[38]  = CC_SPL;   /* & */
  normal[39]  = OPTIONAL_CHAR; /* ' */
  normal[40]  = OPTIONAL_CHAR;   /* ( */
  normal[41]  = OPTIONAL_CHAR;   /* ) */
  //normal[42]  = CC_SPL;   /* * */
  //normal[43]  = CC_SPL;   /* + */
  normal[44]  = '_'; /* , */
  normal[45]  = '_';   /* - */
  normal[46]  = '_'; /* . */
  normal[47]  = '_';   /* / */
  //normal[48]  = CC_DEC;   /* 0 */
  //normal[49]  = CC_DEC;   /* 1 */
  //normal[50]  = CC_DEC;   /* 2 */
  //normal[51]  = CC_DEC;   /* 3 */
  //normal[52]  = CC_DEC;   /* 4 */
  //normal[53]  = CC_DEC;   /* 5 */
  //normal[54]  = CC_DEC;   /* 6 */
  //normal[55]  = CC_DEC;   /* 7 */
  //normal[56]  = CC_DEC;   /* 8 */
  //normal[57]  = CC_DEC;   /* 9 */
  normal[58]  = '_'; /* : */
  //normal[59]  = CC_PUNCT; /* ; */
  //normal[60]  = CC_SPL;   /* < */
  //normal[61]  = CC_SPL;   /* = */
  //normal[62]  = CC_SPL;   /* > */
  normal[63]  = '?'; /* ? */
  normal[64]  = '_';   /* @ */
  //normal[65]  = CC_ALPHA; /* A */
  //normal[66]  = CC_ALPHA; /* B */
  //normal[67]  = CC_ALPHA; /* C */
  //normal[68]  = CC_ALPHA; /* D */
  //normal[69]  = CC_ALPHA; /* E */
  //normal[70]  = CC_ALPHA; /* F */
  //normal[71]  = CC_ALPHA; /* G */
  //normal[72]  = CC_ALPHA; /* H */
  //normal[73]  = CC_ALPHA; /* I */
  //normal[74]  = CC_ALPHA; /* J */
  //normal[75]  = CC_ALPHA; /* K */
  //normal[76]  = CC_ALPHA; /* L */
  //normal[77]  = CC_ALPHA; /* M */
  //normal[78]  = CC_ALPHA; /* N */
  //normal[79]  = CC_ALPHA; /* O */
  //normal[80]  = CC_ALPHA; /* P */
  //normal[81]  = CC_ALPHA; /* Q */
  //normal[82]  = CC_ALPHA; /* R */
  //normal[83]  = CC_ALPHA; /* S */
  //normal[84]  = CC_ALPHA; /* T */
  //normal[85]  = CC_ALPHA; /* U */
  //normal[86]  = CC_ALPHA; /* V */
  //normal[87]  = CC_ALPHA; /* W */
  //normal[88]  = CC_ALPHA; /* X */
  //normal[89]  = CC_ALPHA; /* Y */
  //normal[90]  = CC_ALPHA; /* Z */
  //normal[91]  = '_'; /* [ */
  normal[92]  = OPTIONAL_CHAR; /* \ */
  //normal[93]  = CC_SPL; /* ] */
  //normal[94]  = CC_SPL; /* ^ */
  //normal[95]  = CC_SPL; /* _ */
  normal[96]  = OPTIONAL_CHAR; /* ` */
  normal[97]  = 'A'; /* a */
  normal[98]  = 'B'; /* b */
  normal[99]  = 'C'; /* c */
  normal[100] = 'D'; /* d */
  normal[101] = 'E'; /* e */
  normal[102] = 'F'; /* f */
  normal[103] = 'G'; /* g */
  normal[104] = 'H'; /* h */
  normal[105] = 'I'; /* i */
  normal[106] = 'J'; /* j */
  normal[107] = 'K'; /* k */
  normal[108] = 'L'; /* l */
  normal[109] = 'M'; /* m */
  normal[110] = 'N'; /* n */
  normal[111] = 'O'; /* o */
  normal[112] = 'P'; /* p */
  normal[113] = 'Q'; /* q */
  normal[114] = 'R'; /* r */
  normal[115] = 'S'; /* s */
  normal[116] = 'T'; /* t */
  normal[117] = 'U'; /* u */
  normal[118] = 'V'; /* v */
  normal[119] = 'W'; /* w */
  normal[120] = 'X'; /* x */
  normal[121] = 'Y'; /* y */
  normal[122] = 'Z'; /* z */
  normal[123] = OPTIONAL_CHAR; /* { */
  //normal[124] = CC_SPL; /* | */
  normal[125] = OPTIONAL_CHAR; /* } */
  //normal[126] = CC_SPL; /* ~ */
  normal[127] = OPTIONAL_CHAR; /* DEL */

  init();
}

char TextMetric::
getnorm(char c) const {
  if (isspace(c))
	return '_';
  else if (islower(c))
	return toupper(c);
  else return c;
};

int TextMetric::
geticost(char sc) const {
  int cost = 0;
  if (sc == OPTIONAL_CHAR)
	return 0;
  switch (cc[sc]) {
	case CC_BINARY:
	  cost = DEFAULT_COST; break;
	case CC_CTRL:
	  cost = DEFAULT_COST; break;
	case CC_SPACE:
	  cost = scale(0.75, DEFAULT_COST); break;
	case CC_PUNCT:
	  cost = scale(0.75, DEFAULT_COST); break;
	case CC_QUOTE:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_SPL:
	  cost = scale(0.75, DEFAULT_COST); break;
	case CC_ESC:
	  cost = scale(0.2, DEFAULT_COST); break;
	case CC_DEC:
	  cost = DEFAULT_COST; break;
	case CC_ALPHA:
	  cost = DEFAULT_COST; break;
	default:
	  break;
  }
  return cost;
}

int TextMetric::
getdcost(char pc) const {
  int cost = 0;
  if (pc == OPTIONAL_CHAR)
	return 0;
  switch (cc[pc]) {
	case CC_BINARY:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_CTRL:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_SPACE:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_PUNCT:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_QUOTE:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_SPL:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_ESC:
	  cost = scale(0.25, DEFAULT_COST); break;
	case CC_DEC:
	  cost = DEFAULT_COST; break;
	case CC_ALPHA:
	  cost = DEFAULT_COST; break;
	default:
	  break;
  }
  return cost;
}

int TextMetric::
getscost(char pc, char sc) const {
  int cost;

  if ((pc == sc) || (pc == WILD_CARD_CHAR) || (sc == WILD_CARD_CHAR))
	return 0;
  else if (isalpha(pc) && isalpha(sc) && (tolower(pc) == tolower(sc)))
	return scale(0.1, DEFAULT_COST);
  else if (cc[pc] == cc[sc]) {
	if ((cc[pc] == CC_SPACE) || (cc[pc] == CC_QUOTE))
	  return scale(0.1, DEFAULT_COST);
  }
  return ( (geticost(sc) + getdcost(pc)) * 3) / 4;
}

BinMetric::BinMetric(): DistMetric() {
  init();
}

char BinMetric::
getnorm(char c) const {
  return c;
};

int BinMetric::
geticost(char sc) const {
  return DEFAULT_COST;
  //return 250;
}

int BinMetric::
getdcost(char pc) const {
  return DEFAULT_COST;
  //return 1;
}

int BinMetric::
getscost(char pc, char sc) const {
  if (pc == sc)
	return 0;
  else //return (geticost(sc) + getdcost(pc));
	return ( (geticost(sc) + getdcost(pc)) * 3) / 4;
}
