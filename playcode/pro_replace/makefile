.PHONY:all
all: mypro mycmd

mypro:repro.c
	gcc -o $@ $^ -std=c99 
mycmd:mycmd.c
	gcc -o $@ $^ -std=c99

.PHONY:clean
clean:
	rm -f mypro mycmd
