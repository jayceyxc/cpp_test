SHELL = /bin/sh
CC = g++
RM = rm

INCLUDES = -I./include -I./src -I./include/botan -I/usr/local/include/boost
LIB_PATH = -Wl,-static -L./lib
LDFLAGS = -Wl,-static -lbotan -Wl,-static -lboost_system-mt -Wl,-static -lboost_filesystem-mt -Wl,-static -lboost_regex-mt

CPPFLAGS = -g -Wall --std=c++0x

MOBJ = src/main.o
OBJS = src/person.o src/http_client.o src/push_id_kit.o

MAIN = main

all: $(MAIN)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

$(MAIN): $(MOBJ) $(OBJS)
	$(CC) -o $@ $^ $(LIB_PATH) $(LDFLAGS)

clean:
	$(RM) -f $(MOBJ) $(OBJS) $(MAIN)
