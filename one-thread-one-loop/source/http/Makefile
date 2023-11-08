CXX = g++
DEBUG = -g
THREAD = -lpthread
HIGH_VERSION = -std=c++17
CXXFLAGS = -c #-Wall

TARGET = server
OBJ = $(patsubst %.cc, %.o, $(wildcard $(TARGET).cc))
HEADERS = httpserver.hpp ../server.hpp

$(TARGET) : $(OBJ)
	$(CXX) -o $@ $^ $(HIGH_VERSION) $(THREAD) $(DEBUG)

%.o : %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(DEBUG)

.PHONY:clean
clean:
	rm -rf $(TARGET) *.o