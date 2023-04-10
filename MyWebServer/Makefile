CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = server

OBJS = *.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET) -pthread -lmysqlclient

clean:
	rm -rf server