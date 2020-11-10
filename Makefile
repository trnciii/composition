SRCDIR = ./src
OUTDIR = ./build
LIBS = ./lib_static

ifeq ($(OS),Windows_NT)

# variations
blend: $(SRCDIR)/interface.cpp $(OUTDIR)/cmp.o
	g++ -o $(OUTDIR)/composition.pyd -shared $(OUTDIR)/cmp.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17	-I /mingw64/include/python3.8\
	 -static $(LIBS)/libboost_python37-mgw10-mt-x64-1_74.a $(LIBS)/python37.dll

py: $(SRCDIR)/interface.cpp cmp.o
	g++ -o $(OUTDIR)/composition.pyd -shared -fPIC $(OUTDIR)/cmp.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17 -I /mingw64/include/python3.8 -lboost_python38-mt -lpython3.8

cpp: $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp
	g++ -o $(OUTDIR)/main.exe $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17

# object
$(OUTDIR)/cmp.o: $(SRCDIR)/cmp.cpp
	g++ -c -o $(OUTDIR)/cmp.o  $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17


clean:
	$(RM) $(OUTDIR)/composition.pyd
	$(RM) $(OUTDIR)/main.exe
	$(RM) $(OUTDIR)/cmp.o

# windows
endif


UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
# variations
py: $(SRCDIR)/interface.cpp $(SRCDIR)/cmp.cpp
	g++ -o $(OUTDIR)/composition.so -shared -fPIC $(SRCDIR)/interface.cpp $(SRCDIR)/cmp.cpp\
	 -O3 -fopenmp -std=c++17 -I /usr/include/python3.8 -lboost_python38 -lpython3.8

cpp: $(SRCDIR)/main.cpp
	g++ -o $(OUTDIR)/main $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17


clean:
	$(RM) $(OUTDIR)/composition.so
	$(RM) $(OUTDIR)/main
	$(RM) $(OUTDIR)/cmp.o

# linux
endif