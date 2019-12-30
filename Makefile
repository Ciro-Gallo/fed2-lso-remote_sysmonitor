
all: server client agent

agent: agent.o utility.o 
	gcc -o run_agent agent.o utility.o 

agent.o: agent/agent.c
	gcc -g -Wall -c agent/agent.c

client: client.o client_utility.o utility.o
	gcc -o run_client client.o client_utility.o utility.o

client.o: client/client.c 
	gcc -g -Wall -c client/client.c

client_utility.o: client/client_utility.c client/client_utility.h
	gcc -g -Wall -c client/client_utility.c

server: server.o server_utility.o bst.o list.o utility.o
	gcc -pthread -o run_server server.o server_utility.o bst.o list.o utility.o

server.o: server/server.c 
	gcc -g -Wall -c server/server.c 

server_utility.o: server/server_utility.c server/server_utility.h
	gcc -g -Wall -c server/server_utility.c 

bst.o: bst/bst.c bst/bst.h
	gcc -g -Wall -c bst/bst.c 

list.o: list/list.c list/list.h
	gcc -g -Wall -c list/list.c list/list.h

utility.o: utility/utility.c utility/utility.h
	gcc -g -Wall -c utility/utility.c

clean: 
	rm *.o run_server run_client run_agent