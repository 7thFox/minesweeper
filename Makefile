bin/:
	mkdir bin

build: main.cpp bin/
	g++ main.cpp -o bin/minesweeper -lncurses 

run: build
	bin/minesweeper

clean:
	rm -rf bin
