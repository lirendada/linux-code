vpath %.h ..
objects = main.o command.o display.o defs.o

edit : $(objects)
	gcc -o edit $(objects)

main.o : main.c
	gcc -c $< -I ../
command.o : command.c command.h
	gcc -c $< -I ../
defs.o : defs.c defs.h
	gcc -c $< -I ../
display.o : display.c display.h
	gcc -c $< -I../

clean : 
	-rm -f edit *.o