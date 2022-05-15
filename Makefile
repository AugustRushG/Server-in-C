EXE_PHASE1=server
server: server.c
	gcc -pthread -o server server.c -Wall

clean :
	rm -f *.o $(EXE_PHASE1)