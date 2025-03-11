CXX = g++
#CFLAGS = -std=c++14 -O2 -Wall -g 
CFLAGS = -Wall -g
INCLUDES = -I$(CURDIR)/code/log
TARGET = radar_server
OBJS = code/*.cpp code/config/*.cpp \
       code/app/*.cpp code/server/*.cpp \
       code/buffer/*.cpp process/*.cpp websocket/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(INCLUDES) -o $(TARGET)  -pthread -lmysqlclient -lrt

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




