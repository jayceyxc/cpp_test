SHELL = /bin/sh
CC = g++
RM = rm

INCLUDES = -I./include -I./src -I/usr/local/include/boost -I/usr/include/hiredis/
LIB_PATH = -L/usr/local/lib
LDFLAGS = -lboost_system -lboost_filesystem -lboost_regex -lhiredis
LDFLAGS = -Wl,-static \
		  -Wl,-static -lboost_thread \
		  -Wl,-static -lboost_program_options \
		  -Wl,-static -lboost_system \
		  -Wl,-static -lboost_filesystem \
		  -Wl,-static -lboost_regex \
		  -Wl,-static -lhiredis \
		  -Wl,-static -lpthread -lrt \
		  -Wl,-static -ldl \
		  -static -static-libgcc

CPPFLAGS = -g -Wall --std=c++0x

MOBJ = src/main.o
RADIUS_OBJ = src/radius_client.o src/redis.o
OBJS = src/person.o src/http_client.o

MAIN = main
RADIUS_CLIENT = radius_client

all: $(MAIN) $(RADIUS_CLIENT)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

$(MAIN): $(MOBJ) $(OBJS)
	$(CC) -o $@ $^ $(LIB_PATH) $(LDFLAGS)

$(RADIUS_CLIENT): $(RADIUS_OBJ)
	$(CC) -o $@ $^ $(LIB_PATH) $(LDFLAGS)

clean:
	$(RM) -f $(MOBJ) $(OBJS) $(MAIN) $(RADIUS_OBJ) $(RADIUS_CLIENT)
