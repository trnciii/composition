SRCDIR = ./src
OUTDIR = ./build
LIBS = ./lib_static

ifeq ($(OS),Windows_NT)

blend: $(OUTDIR)/cmp_blend.pyd
	cp $(OUTDIR)/cmp_blend.pyd ./composition/core/composition.pyd

py: $(OUTDIR)/cmp_py.pyd
	cp $(OUTDIR)/cmp_py.pyd composition.pyd

cpp: $(OUTDIR)/main.exe
	cp $(OUTDIR)/main.exe main.exe

# objects
$(OUTDIR)/cmp_blend.pyd: $(SRCDIR)/interface.cpp $(OUTDIR)/cmp.o
	g++ -o $(OUTDIR)/cmp_blend.pyd -shared $(OUTDIR)/cmp.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17	-I /mingw64/include/python3.8\
	 -static $(LIBS)/libboost_python37-mgw10-mt-x64-1_74.a $(LIBS)/python37.dll

$(OUTDIR)/cmp_py.pyd: $(SRCDIR)/interface.cpp $(OUTDIR)/cmp.o
	g++ -o $(OUTDIR)/cmp_py.pyd -shared -fPIC $(OUTDIR)/cmp.o $(SRCDIR)/interface.cpp\
	 -O3 -fopenmp -std=c++17 -I /mingw64/include/python3.8 -lboost_python38-mt -lpython3.8

$(OUTDIR)/main.exe: $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp
	g++ -o $(OUTDIR)/main.exe $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17


$(OUTDIR)/cmp.o: $(SRCDIR)/cmp.cpp
	g++ -c -o $(OUTDIR)/cmp.o  $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17

.PHONY: clean
clean:
	$(RM) $(OUTDIR)/cmp_blend.pyd
	$(RM) $(OUTDIR)/cmp_py.pyd
	$(RM) $(OUTDIR)/main.exe
	$(RM) $(OUTDIR)/cmp.o
	$(RM) composition.pyd
	$(RM) main.exe

# windows
endif


UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
# variations
py: $(OUTDIR)/cmp_py.so
	cp $(OUTDIR)/cmp_py.so ./composition/core/composition.so

cpp: $(OUTDIR)/main
	cp $(OUTDIR)/main main

# objects
$(OUTDIR)/cmp_py.so: $(SRCDIR)/interface.cpp $(SRCDIR)/cmp.cpp
	g++ -o $(OUTDIR)/cmp_py.so -shared -fPIC $(SRCDIR)/interface.cpp $(SRCDIR)/cmp.cpp\
	 -O3 -fopenmp -std=c++17 -I /usr/include/python3.8 -lboost_python38 -lpython3.8

$(OUTDIR)/main: $(SRCDIR)/main.cpp
	g++ -o $(OUTDIR)/main $(SRCDIR)/main.cpp $(SRCDIR)/cmp.cpp -O3 -fopenmp -std=c++17


.PHONY: clean
clean:
	$(RM) $(OUTDIR)/cmp_py.so
	$(RM) $(OUTDIR)/main
	$(RM) composition.so
	$(RM) main

# linux
endif