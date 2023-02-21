.PHONY : all
all : shm_client shm_server

shm_client : client.cpp
	g++ -o $@ $^ -std=c++11
shm_server : server.cpp
	g++ -o $@ $^ -std=c++11

.PHONY : clean
clean : 
	rm -f shm_client shm_server