.PHONY:gobang
gobang:gobang.cc logger.hpp util.hpp db.hpp online.hpp
	g++ -o$@ $^ -std=c++11 -L/usr/lib64/mysql/ -lmysqlclient -ljsoncpp -lboost_system -lpthread

.PHONY:clean
clean:
	rm -f gobang