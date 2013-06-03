all: example

example: main.o fmod.o
	gcc -m64 main.o fmod.o -o example ../../api/lib/libfmodex64.so

main.o: main.c
	gcc -m64 -c main.c

fmod.o: fmod.c
	gcc -m64 -c fmod.c

clean:
	rm -f example
