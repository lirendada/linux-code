.PHONY:all 
all:	mysql json server

mysql:mysql.c
	gcc -o $@ $^ -L/usr/lib64/mysql/ -lmysqlclient
json:json.cpp
	g++ -o $@ $^ -std=c++11 -lboost_system -lpthread -ljsoncpp
server:wsserver.cpp
	g++ -o $@ $^ -std=c++11 -lboost_system -lpthread

.PHONY:clean
clean:
	rm -f server mysql json
