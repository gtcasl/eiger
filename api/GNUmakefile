# example makefile for the scons-less and those without stock C++ mysql bindings
# you will likely need to edit the variable definitions below unless you are baallan
# or are running a similar ubuntu environment.
# TOOLS=(gcc|intel)
TOOLS=gcc

ifeq ($(TOOLS),intel)
CXX=icpc 
MPICXX=mpicxx
endif

ifeq ($(TOOLS),gcc)
CXX=g++
MPICXX=mpicxx.openmpi
endif


CXXFLAGS= -std=c++0x -fPIC -I.. -g

MYSQLINC=-I/usr/include/mysql++ -I/usr/include/mysql
ifeq ($(USER),baallan)
MYSQLINC=-I/home/baallan/sst/mysqlpp/include/mysql++ -I/usr/include/mysql
endif

all: libs loader

libs: dirs
	$(CXX) -o eiger.os -c $(CXXFLAGS)  $(MYSQLINC) eiger.cpp
	$(CXX) -o $(TOOLS)/libeigerInterface.so -shared eiger.os -L.
	ar crv $(TOOLS)/libeigerInterface.a eiger.os 
	$(CXX) -o fakeeiger.os -c $(CXXFLAGS)  fakeeiger.cpp
	$(CXX) -o $(TOOLS)/libfakeeigerInterface.so -shared fakeeiger.os -L.
	$(MPICXX) -o mpifakeeiger.os -c $(CXXFLAGS)  mpifakeeiger.cpp
	$(CXX) -o $(TOOLS)/libmpifakeeigerInterface.so -shared mpifakeeiger.os -L.

loader:
	$(CXX) -o $(TOOLS)/feloader $(CXXFLAGS) feloader.cpp -L$(TOOLS) -leigerInterface -lmysqlpp -Wl,-rpath,`pwd`/$(TOOLS)
	$(CXX) -o $(TOOLS)/mfeloader $(CXXFLAGS) mfeloader.cpp -L$(TOOLS) -leigerInterface -lmysqlpp -Wl,-rpath,`pwd`/$(TOOLS)

dirs:
	-mkdir $(TOOLS)

clean:
	$(RM) -rf $(TOOLS) *.o *.os feloader lib*.so mfeloader
