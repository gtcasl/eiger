###################################################################
#  USER-CONFIGURABLE PARAMETERS
###################################################################
EIGERINCLUDE = /usr/local/include/eiger
EIGERLIBDIR = /usr/local/lib
LWPDIR = $(HOME)/eiger/lwperf/src

###################################################################
#  DEFAULT PARAMETERS - Shouldn't need to edit
###################################################################
TARGET = HPCCG.x
SRC := $(shell ls *.cpp) 

CXX=mpicxx
LWPINCS = -I$(LWPDIR) -I.
CPPFLAGS = -O2 $(LWPINCS)
LDFLAGS = -O2
SETCAP = 

ifeq ($(SSTMAC),y)
SRC := $(shell ls *.cpp) valid_keywords.cc
CXX :=      $(shell sstmacro-config --cxx )
CC :=      $(shell sstmacro-config --cc )
CXXFLAGS := $(shell sstmacro-config --cxxflags )
CPPFLAGS := $(shell sstmacro-config --cppflags ) -DUSING_SSTMAC $(LWPINCS)
LIBDIR :=   $(shell sstmacro-config --libdir )
PREFIX :=   $(shell sstmacro-config --prefix )
LDFLAGS :=  $(shell sstmacro-config --ldflags )  -Wl,-rpath,$(PREFIX)/lib
SSTGEN = valid_keywords.cc
endif

# set profiling option. default is off. Settable from command line as
# make clean; make LWPERF=some_value
LWPERF =EIGER
LWPERF =FAKEEIGER
LWPERF =CSV
LWPERF =

# compute perf files and rules
ifneq ($(LWPERF),)
LWTOOLINC += -I$(EIGERINCLUDE)
LWPGEN = InitFuncs.h LocationElements.h InitSwitchElements.h cperf._log.h cperf._save.h cperf._stop.h
LWPSRC = perf.cpp

ifeq ($(LWPERF),FAKEEIGER)
ifeq ($(LWP_DBNAME),)
$(error need to provide a LWP_DBNAME)
endif
LWPSRC += eigerbackend.cpp 
EIGERLIB = -L$(EIGERLIBDIR) -lfakeeigerInterface -lpapi
LWTOOLINC += -D_USE_FAKEEIGER -DLWP_DBNAME='"$(LWP_DBNAME)"'
SETCAP = sudo setcap cap_sys_rawio=ep $(TARGET)
endif

ifeq ($(LWPERF),MODEL)
LWTOOLINC += -D_USE_EIGER_MODEL -DLWP_DBNAME='"tmp"'
endif

LWPOBJ = $(LWPSRC:.cpp=.o)
endif

CXXFLAGS += $(LWTOOLINC)
LIBS += $(EIGERLIB)
SRC += $(LWPSRC)
OBJ := $(SRC:.cc=.o) 
OBJ := $(OBJ:.cpp=.o)
OBJ := $(OBJ:.cxx=.o)
OBJ := $(OBJ:.c=.o)

VPATH += $(LWPDIR)

.PHONY: clean install fake cleanfake sstmac cleansstmac

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CXX) -o $@ $+ $(LDFLAGS) $(LIBS) 
	$(SETCAP)

%.o: %.cpp $(LWPGEN)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean: 
	$(RM) $(TARGET) $(OBJ) $(LWPOBJ) $(LWPGEN) *_mpi_*.log $(SSTGEN)

install: $(TARGET)
	cp $< $(PREFIX)/bin

$(LWPGEN): $(SRC)
	$(LWPDIR)/updateLoggersCXX.sh $(SRC)

valid_keywords.cc:
	sst_parse_keywords -t register -n external

# helper rules
instrument:
	$(MAKE) LWPERF=FAKEEIGER

cleaninstrument:
	$(MAKE) LWPERF=FAKEEIGER LWP_DBNAME=tmp clean

sstmac:
	$(MAKE) SSTMAC=y

cleansstmac:
	$(MAKE) SSTMAC=y clean

eiger:
	$(MAKE) SSTMAC=y LWPERF=MODEL

cleaneiger:
	$(MAKE) SSTMAC=y LWPERF=MODEL clean
