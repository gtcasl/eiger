# Makefile for mpicxx compiler

SHELL = /bin/sh
#.IGNORE:

OBJ1 = $(SRC:.cpp=.o)
OBJ2 = $(OBJ1:.c=.o)
OBJ = $(OBJ2:.F90=.o)

# load in eiger bits
# intel, gcc
TOOLS=gcc
include Makefile.mpieiger

# System-specific settings and -g/-O opts
DEB=

#CC =		mpicc.openmpi
#CXX=		mpicxx.openmpi
FC = 		mpif90.openmpi
CFLAGS =	$(DEB) 
CCFLAGS =	$(DEB) -DMPICH_IGNORE_CXX_SEEK -D_USE_EIGER 
LINK =		mpif90.openmpi
LINKFLAGS =	$(DEB)
USRLIB =	-lmpi_cxx -lstdc++
SYSLIB =	$(EIGER_LIB)
SIZE =		size

# Link rule

$(EXE):	$(OBJ)
	$(LINK) $(LINKFLAGS) $(OBJ) $(USRLIB) $(SYSLIB) -o $(EXE)
	$(SIZE) $(EXE)

# Compilation rules
.SUFFIXES:
.SUFFIXES: .cpp .cxx .c .h .F90 .o

.c.o:
	$(CC) $(CFLAGS) -c  $<

.cpp.o:
	$(CXX) $(CCFLAGS) -c $(CXXFLAGS) $<

.F90.o:
	$(FC) $(FCFLAGS) -c $(CXXFLAGS) $<

# Individual dependencies

$(OBJ): $(INC)

# fortran module dependence listed explicitly to enable pll make
app.o: fperf.o