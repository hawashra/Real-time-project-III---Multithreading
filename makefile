run: compile
	@./main user-defined.txt

compile:
	@gcc -o main main.c -lpthread
	@gcc -o production_line production_line.c -lpthread

clean:
	@rm -f main production_line