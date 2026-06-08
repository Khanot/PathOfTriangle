CC=gcc -g
PROGRAMS=rm pm

all: $(PROGRAMS)

Graph.o: Graph.c Graph.h
	$(CC) -c Graph.c -Wall

Algo.o: Algo.c Algo.h
	$(CC) -c Algo.c -Wall

Matching.o: Matching.c Matching.h
	$(CC) -c Matching.c -Wall

Utils.o: Utils.c Utils.h
	$(CC) -c Utils.c -Wall

PathOfTriangleCHeck.o: PathOfTriangleCHeck.c PathOfTriangleCHeck.h
	$(CC) -c PathOfTriangleCHeck.c -Wall

randomMain.o: randomMain.c
	$(CC) -c randomMain.c -Wall

parsingMain.o: parsingMain.c
	$(CC) -c parsingMain.c -Wall

rm:  Graph.o Algo.o Matching.o Utils.o PathOfTriangleCHeck.o randomMain.o 
	$(CC)  Graph.o Algo.o Matching.o Utils.o PathOfTriangleCHeck.o randomMain.o -o rm -lm -Wall

pm:  Graph.o Algo.o Matching.o Utils.o PathOfTriangleCHeck.o parsingMain.o 
	$(CC)  Graph.o Algo.o Matching.o Utils.o PathOfTriangleCHeck.o parsingMain.o -o pm -lm -Wall


	
clean:
	rm -f $(PROGRAMS) *.o