myproc:main.o proc.o
		gcc -o $@ $^
main.o:main.c
		gcc -c $<
proc.o:proc.c
		gcc -c $<

.PHONY:clean
clean:
		rm -f *.o myproc
