ifeq ($(OS),Windows_NT)
composition.pyd: interface.cpp cmp.o
	g++ -o composition.pyd -shared -fPIC cmp.o interface.cpp -O3 -fopenmp -std=c++17 -I /mingw64/include/python3.8 -lboost_python38-mt -lpython3.8

main.exe: cmp.cpp cmp.o
	g++ -o main.exe main.cpp cmp.o -O3 -fopenmp -std=c++17

cmp.o:cmp.cpp
	g++ -o cmp.so cmp.cpp -O3 -fopenmp -std=c++17

clean:
	$(RM) composition.pyd
	$(RM) main.exe
	$(RM) cmp.o

# windows
endif


UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
composition.so: interface.cpp cmp.cpp
	g++ -o composition.so -shared -fPIC interface.cpp cmp.cpp -O3 -fopenmp -std=c++17 -I /usr/include/python3.8 -lboost_python38 -lpython3.8

main: main.cpp cmp.o
	g++ -o main main.cpp cmp.cpp -O3 -fopenmp -std=c++17

clean:
	$(RM) composition.so
	$(RM) main
	$(RM) cmp.o

# linux
endif