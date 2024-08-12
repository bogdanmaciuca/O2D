all: glad.o o2d demo

glad.o:
	gcc -c -o glad.o ../src/vendor/glad.c

o2d:
	gcc -c -o o2d.o ../src/o2d.c
	ar rcs libo2d.a o2d.o glad.o temp/*.obj

demo:
	gcc -o demo ../demo/demo.c -L. -lo2d -lopengl32 -luser32 -lgdi32
