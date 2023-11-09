CXX = g++
TARGET = client2
OBJ = $(patsubst %.cc, %.o, $(wildcard $(TARGET).cc))

CXXFLAGS = -c #-Wall

$(TARGET) : $(OBJ)
	$(CXX) -o $@ $^ -std=c++11

%.o : %.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY:clean
clean:
	rm -f $(OBJ) $(TARGET)