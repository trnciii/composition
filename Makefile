SRCDIR = ./src
OUTDIR = ./build
LIBS = ./lib_static
ADDON = ./composition/core/composition.pyd

# only for msys
ifeq ($(OS),Windows_NT)

# lib
.PHONY: composition
composition: $(SRCDIR)/interface.cpp $(OUTDIR)/composition.o
	g++ -o $(ADDON) -shared $(OUTDIR)/composition.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17	-I /mingw64/include/python3.8\
	 -static $(LIBS)/libboost_python37-mgw10-mt-x64-1_74.a $(LIBS)/python37.dll

$(OUTDIR)/composition.o: $(SRCDIR)/composition.cpp
	g++ -c -o $(OUTDIR)/composition.o  $(SRCDIR)/composition.cpp -O3 -fopenmp -std=c++17

# exe
main: $(SRCDIR)/main.cpp $(SRCDIR)/composition.cpp
	g++ -o $(OUTDIR)/main.exe $(SRCDIR)/main.cpp $(SRCDIR)/composition.cpp -O3 -fopenmp -std=c++17

endif