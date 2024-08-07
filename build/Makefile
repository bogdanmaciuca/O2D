all: o2d demo

o2d:
	gcc -c -o o2d.o ../o2d.c
	gcc -c -o glad.o ../glad.c
	ar rcs libo2d.a o2d.o glad.o temp/*.obj

demo:
	gcc -o demo ../demo.c -L. -lo2d -lopengl32 -luser32 -lgdi32
