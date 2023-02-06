.PHONY : all
all : server client

server : server.cpp
	g++ -o $@ $^ -std=c++11 -g

client : client.cpp
	g++ -o $@ $^ -std=c++11 -g 

.PHONY : clean
clean:	
	rm -f server client