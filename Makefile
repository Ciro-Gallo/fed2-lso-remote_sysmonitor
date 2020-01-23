
#VARIABLES

CC = gcc
CFLAGS = -g -Wall -c

#TARGETS DEFINITION

all: server client agent

agent: agent.o utility.o 
	$(CC) -o run_agent agent.o utility.o 

agent.o: agent/agent.c
	$(CC) $(CFLAGS) agent/agent.c

client: client.o client_utility.o utility.o
	$(CC) -o run_client client.o client_utility.o utility.o

client.o: client/client.c 
	$(CC) $(CFLAGS) client/client.c

client_utility.o: client/client_utility.c client/client_utility.h
	$(CC) $(CFLAGS) client/client_utility.c

server: server.o server_utility.o bst.o list.o utility.o
	$(CC) -pthread -o run_server server.o server_utility.o bst.o list.o utility.o

server.o: server/server.c 
	$(CC) $(CFLAGS) server/server.c 

server_utility.o: server/server_utility.c server/server_utility.h
	$(CC) $(CFLAGS) server/server_utility.c 

bst.o: bst/bst.c bst/bst.h
	$(CC) $(CFLAGS) bst/bst.c 

list.o: list/list.c list/list.h
	$(CC) $(CFLAGS) list/list.c list/list.h

utility.o: utility/utility.c utility/utility.h
	$(CC) $(CFLAGS) utility/utility.c

clean: 
	rm *.o run_server run_client run_agent