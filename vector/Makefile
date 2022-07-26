vector:vector.o
	g++ -o $@ $^
vector.o:vector.cpp
	g++ -c $< -std=c++11

.PHONY:clean
clean:
	rm -f *.o vector
