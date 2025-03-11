static: stringlibrary.c
	gcc -c stringlibrary.c
	ar rcs libstringlibrary.a stringlibrary.o
	gcc main.c -o main_static -L. -lstringlibrary

shared: stringlibrary.c 
	gcc -c -fPIC stringlibrary.c
	gcc -shared -o libstringlibrary.so stringlibrary.o
	gcc main.c -L. -lstringlibrary -o main_shared

clean:
	rm -f main *.o *.a *.so

.PHONY: clean static shared