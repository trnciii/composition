SRCDIR = ./src
OUTDIR = ./build
LIBS = ./lib_static
ADDON = ./composition/core/composition.pyd

# only for msys
ifeq ($(OS),Windows_NT)

# lib
.PHONY: composition
composition: $(SRCDIR)/interface.cpp $(OUTDIR)/cmp.o
	g++ -o $(ADDON) -shared $(OUTDIR)/cmp.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17	-I /mingw64/include/python3.8\
	 -static $(LIBS)/libboost_python37-mgw10-mt-x64-1_74.a $(LIBS)/python37.dll

$(OUTDIR)/cmp.o: $(SRCDIR)/cmp.cpp
	g++ -c -o $(OUTDIR)/cmp.o  $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17

# exe
.PHONY: main
main: $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp
	g++ -o $(OUTDIR)/main.exe $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17

endif