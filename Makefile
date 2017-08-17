SHELL = /bin/sh
CC = g++
RM = rm

INCLUDES = -I./include -I./src -I/usr/local/include/boost
LIB_PATH =
LDFLAGS = -lboost_system-mt -lboost_filesystem-mt -lboost_regex-mt

CPPFLAGS = -g -Wall -pedantic --std=c++0x

MOBJ = src/main.o
OBJS = src/person.o src/http_client.o

MAIN = main

all: $(MAIN)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

$(MAIN): $(MOBJ) $(OBJS)
	$(CC) -o $@ $^ $(LIB_PATH) $(LDFLAGS)

clean:
	$(RM) -f $(MOBJ) $(OBJS) $(MAIN)
