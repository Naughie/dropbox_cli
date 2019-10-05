CC = g++
CCOPTS = -std=c++17 -o $(EXEC)
LCURL = -lcurl
SRC = ./src
CSRC = $(SRC)/c/*.cpp
CINC = $(SRC)/c
INCLUDE = -I$(CINC)
BIN = ./bin
EXEC = $(BIN)/dropbox

c:
	$(CC) $(CCOPTS) $(INCLUDE) $(wildcard $(CSRC)) $(LCURL)
