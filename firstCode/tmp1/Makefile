mytest:test.o main.o
		gcc -o $@ $^
test.o:test.c
		gcc -c $<
main.o:main.c
		gcc -c $<

.PHONY:clean
clean:
		rm -f *.o mytest

