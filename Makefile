ifeq ($(OS),Windows_NT)
composition.pyd: interface.cpp cmp.o
	g++ -o composition.pyd -shared cmp.o interface.cpp -O3 -fopenmp -std=c++17 -static -I /mingw64/include/python3.8 libboost_python37-mgw10-mt-x64-1_74.a python37.dll

main.exe: main.cpp cmp.cpp
	g++ -o main main.cpp cmp.cpp -O3 -fopenmp -std=c++17

cmp.o:cmp.cpp
	g++ -c -o cmp.o cmp.cpp -O3 -fopenmp -std=c++17

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