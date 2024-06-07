run: compile
	@./main user-defined.txt

compile:
	@gcc -o main main.c -lpthread
	@gcc -o production_line production_line.c -lpthread
	@gcc -o OpenGL OpenGL.c -lglut -lGLU -lGL -lm

clean:
	@rm -f main production_line OpenGL