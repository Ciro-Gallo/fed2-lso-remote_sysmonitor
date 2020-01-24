
#The following modularization was designed with the aim of facilitating all future changes.

#VARIABLES

CC = gcc
CFLAGS = -g -Wall -c
PTHREAD_FLAGS = -pthread

FLD_AGENT = agent
BIN_AGENT = agent
OBJ_AGENT = agent.o 
SRC_AGENT = $(FLD_AGENT)/agent.c 

FLD_CLIENT = client
BIN_CLIENT = client
OBJ_CLIENT = client.o 
OBJ_CLIENT_UTILITY = client_utility.o
SRC_CLIENT = $(FLD_CLIENT)/client.c 
SRC_CLIENT_UTILITY = $(FLD_CLIENT)/client_utility.c 
HEADER_CLIENT_UTILITY = $(FLD_CLIENT)/client_utility.h

FLD_SERVER = server
BIN_SERVER = server
OBJ_SERVER = server.o 
OBJ_SERVER_UTILITY = server_utility.o
SRC_SERVER = $(FLD_SERVER)/server.c
SRC_SERVER_UTILITY = $(FLD_SERVER)/server_utility.c
HEADER_SERVER_UTILITY = $(FLD_SERVER)/server_utility.h

OBJ_BST = bst.o 
SRC_BST = bst/bst.c
HEADER_BST = bst/bst.h

OBJ_LIST = list.o 
SRC_LIST = list/list.c 
HEADER_LIST = list/list.h

FLD_UTILITY = utility
OBJ_UTILITY = utility.o
SRC_UTILITY = $(FLD_UTILITY)/utility.c
HEADER_UTILITY = $(FLD_UTILITY)/utility.h


#TARGETS DEFINITION

all: $(BIN_SERVER) $(BIN_CLIENT) $(BIN_AGENT)

$(BIN_AGENT): $(OBJ_AGENT) $(OBJ_UTILITY) 
	$(CC) -o run_agent $^
	@echo Agent compiled.
	@echo ""

$(OBJ_AGENT): $(SRC_AGENT)
	$(CC) $(CFLAGS) $(SRC_AGENT)

$(BIN_CLIENT): $(OBJ_CLIENT) $(OBJ_CLIENT_UTILITY) $(OBJ_UTILITY)
	$(CC) -o run_client $^
	@echo Client compiled.
	@echo ""

$(OBJ_CLIENT): $(SRC_CLIENT)
	$(CC) $(CFLAGS) $^

$(OBJ_CLIENT_UTILITY): $(SRC_CLIENT_UTILITY) $(HEADER_CLIENT_UTILITY)
	$(CC) $(CFLAGS) $(SRC_CLIENT_UTILITY)

$(BIN_SERVER): $(OBJ_SERVER) $(OBJ_SERVER_UTILITY) $(OBJ_BST) $(OBJ_LIST) $(OBJ_UTILITY)
	$(CC) $(PTHREAD_FLAGS) -o run_server $^
	@echo Server compiled.
	@echo ""

$(OBJ_SERVER): $(SRC_SERVER)
	$(CC) $(CFLAGS) $^

$(OBJ_SERVER_UTILITY): $(SRC_SERVER_UTILITY) $(HEADER_SERVER_UTILITY)
	$(CC) $(CFLAGS) $(SRC_SERVER_UTILITY)

$(OBJ_BST): $(SRC_BST) $(HEADER_BST)
	$(CC) $(CFLAGS) $(SRC_BST) 

$(OBJ_LIST): $(SRC_LIST) $(HEADER_LIST)
	$(CC) $(CFLAGS) $(SRC_LIST)

$(OBJ_UTILITY): $(SRC_UTILITY) 
	$(CC) $(CFLAGS) $^


#DEEP CLEAN

clean: cleanObj cleanBin

cleanObj: 
	@echo Cleaning object files:
	rm -f *.o 

cleanBin:
	@echo Cleaning binary files:
	rm -f run_server run_client run_agent