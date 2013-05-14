COMOPTS =
CC = g++
FLEX=flex
BISON=bison
#CCOPTS = -c -g
CCOPTS = -c -g -O2 -m32 -Wno-deprecated

LINKOPTS= -lpcrecpp
#LINKOPTS= -pg -lpcrecpp

monCSRF: monCSRF.o RmProcess.o p1.o DistMetric.o ST.o STE.o sql.lex.o sql.tab.o SyntaxTree.o TreeAuto.o TEPT.o SqlInfo.o
	g++ -o monCSRF monCSRF.o RmProcess.o p1.o DistMetric.o ST.o STE.o sql.lex.o sql.tab.o  SyntaxTree.o  TreeAuto.o TEPT.o SqlInfo.o $(COMOPTS) $(LINKOPTS)

SqlInfo.o: SqlInfo.C SqlInfo.h ParserUtil.h
	g++ ${CCOPTS} ${COMOPTS} SqlInfo.C

TreeAuto.o: TreeAuto.C TreeAuto.h
	g++ $(CCOPTS) $(COMOPTS) TreeAuto.C

p1.o: p1.C p1.h ST.h STE.h
	g++ $(CCOPTS) $(COMOPTS) p1.C

DistMetric.o: DistMetric.C DistMetric.h
	g++ $(CCOPTS) $(COMOPTS) DistMetric.C

monCSRF.o: monCSRF.C
	g++ $(CCOPTS) $(COMOPTS) monCSRF.C

RmProcess.o: RmProcess.C p1.h ST.h STE.h
	g++ $(CCOPTS) $(COMOPTS) RmProcess.C

STE.o: STE.C ST.h STE.h
	g++ $(CCOPTS) $(COMOPTS) STE.C

ST.o: ST.C ST.h STE.h
	g++ $(CCOPTS) $(COMOPTS) ST.C

SyntaxTree.o: SyntaxTree.C  ParserUtil.h
	g++ $(CCOPTS) $(COMOPTS) SyntaxTree.C

TEPT.o: TEPT.C  TEPT.h
	g++ $(CCOPTS) $(COMOPTS) TEPT.C

sql.lex.C: sql.l++ ParserUtil.h
	$(FLEX) -p -8 -Ce -osql.lex.C sql.l++

sql.lex.o:	sql.lex.C sql.tab.C ParserUtil.h
	g++ $(CCOPTS) $(COMOPTS) sql.lex.C

sql.tab.o:	sql.tab.C sql.tab.H ParserUtil.h
	g++ $(CCOPTS) $(COMOPTS) sql.tab.C

sql.tab.C: sql.y ParserUtil.h
	bison -t -d -v -o sql.tab.C sql.y

clean:
	rm -f monCSRF
	rm -f *.o
	rm -f sql.tab.* sql.lex.* sql.output

