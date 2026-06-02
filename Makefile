CC=gcc -g
PROGRAMS=pot

all: $(PROGRAMS)

Graph.o: Graph.c Graph.h
	$(CC) -c Graph.c -Wall


randomMain.o: randomMain.c
	$(CC) -c randomMain.c -Wall



pot:  Graph.o randomMain.o 
	$(CC)  Graph.o randomMain.o -o pot -lm -Wall


	
clean:
	rm -f $(PROGRAMS) *.o