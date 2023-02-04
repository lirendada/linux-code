libmymath.so : add.o sub.o   # 加上-shared封装静态库
	gcc -shared -o $@ $^ 

%.o : %.c    # %的作用是匹配目录下的.c文件集合生成.o文件集合，与*号类似，但是%多用于makefile，且使用范围不太一样
	gcc -fPIC -c $< 
#-fPIC:产生.o目标文件，程序内部的地址方案是：与位置无关，库文件可以在内存的任意位置加载，不影响其他程序的关联性

.PHONY:output      # output作为伪目标进行打包头文件和静态库
output:
	mkdir -p mylib/include
	mkdir -p mylib/lib
	cp -f *.so mylib/lib
	cp -f *.h mylib/include

.PHONY:install
install:
	sudo cp *.h /usr/include
	sudo cp *.so /lib64

.PHONY:clean
clean:
	rm -rf *.o libmymath.so mylib