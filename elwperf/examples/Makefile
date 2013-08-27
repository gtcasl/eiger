# Multiple-configuration Makefile
# As there are lots of variations depending on flag and language,
# we build each one in a subdirectory with it's own makefile.
# All the subdirectories are temporary.
# For a single application, this make complexity is not needed.


SHELL = /bin/sh

# lwperf static Files
CSVSRC= aperf.cpp csvformatter.cpp diffrusage.cpp
CSVINC = lwperf.h aperf.h csvformatter.h diffrusage.h 

EIGERSRC= eperf.cpp eigerformatter.cpp csvformatter.cpp diffrusage.cpp 
EIGERINC = lwperf.h eperf.h eigerformatter.h csvformatter.h diffrusage.h

# c/c++ generated binding/wrapper
BINDGENFILES = cperf._stop.h cperf._log.h cperf._save.h cperf._save.body.h EigerInitFuncs.h InitSwitchElements.h LocationElements.h CSVInitFuncs.h

# sources for applications and the things they need
# additionally, all f90 & c apps will require some of the BINDGENFILES but creating
# these is implied by the language specific genfiles lists.

CXXSRC=cxxapp.cpp
CXXGENFILES = CSVInitFuncs.h EigerInitFuncs.h InitSwitchElements.h LocationElements.h
CXXINC= $(CXXGENFILES)

FSRC = fperf.F90 app.F90 cperf.cpp
FGENFILES = flocations.h fperf._save.h
FINC = $(FGENFILES) lwperf_f.h 

CSRC = capp.c cperf.cpp 
CGENFILES = clocations.h 
CINC = lwperf_c.h $(CGENFILES)

#parallel compile option on multicore hardware.
MAKEPLL=-j8

# make fragments to copy
MAKESUPPORT=Makefile.*

RUNIT= : ooooooooooooooooooooooooooooooooo; echo 2 | mpirun.openmpi -np 2

ALLGEN= $(CXXGENFILES) $(CGENFILES) $(FGENFILES) $(BINDGENFILES) 

# Definitions

ROOT =	cxxapp
EXE =	$(ROOT)_$@
OBJ =	$(CXXSRC:.cpp=.o)

FROOT =	app
FEXE =	$(FROOT)_$@
FOBJ1 =	$(FSRC:.cpp=.o)
FOBJ =	$(FOBJ1:.F90=.o)

CROOT =	capp
CEXE =	$(CROOT)_$@
COBJ1 =	$(CSRC:.cpp=.o)
COBJ =	$(COBJ1:.c=.o)

# Help

help:
	@echo 'Type "make target" where target is one of:'
	@echo '      eiger   (c++ eiger database + csv)'
	@echo '      c       (C eiger database)'
	@echo '      f90     (fortran eiger database)'
	@echo '      csv     (c++ csv only)'
	@echo '      ccsv    (C csv only)'
	@echo '      f90csv  (fortran csv only)'
	@echo '      fake    (c++ text files for eiger loading later + csv)'
	@echo '      cfake   (c text files for eiger loading later + csv)'
	@echo '      f90fake (fortran text files for eiger loading later + csv)'
	@echo '      cscreen (c text files for eiger loading later + csv + screen)'
	@echo '      plain   (c++ no instrumentation)'
	@echo '      cplain  (c no instrumentation)'
	@echo '      fplain  (f90 no instrumentation)'

# Targets

plain: $(CXXSRC) $(CXXINC) $(CSVSRC) $(CSVINC) Make.plain
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CXXSRC) $(CXXINC) $(CSVSRC) $(CSVINC) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(SRC) $(CXXSRC) $(CSVSRC)" "INC = $(INC) $(CXXINC) $(CSVINC)" "EXE = ../$(EXE)" ../$(EXE)


csv: $(CXXSRC) $(CXXINC) $(CSVSRC) $(CSVINC) Make.csv
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CXXSRC) $(CXXINC) $(CSVSRC) $(CSVINC) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(SRC) $(CXXSRC) $(CSVSRC)" "INC = $(INC) $(CXXINC) $(CSVINC)" "EXE = ../$(EXE)" ../$(EXE)


fake:   $(CXXSRC) $(CXXINC) $(EIGERSRC) $(EIGERINC) Make.fake
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CXXSRC) $(CXXINC) $(EIGERSRC) $(EIGERINC) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CXXSRC) $(EIGERSRC)" "INC = $(CXXINC) $(EIGERINC) " "EXE = ../$(EXE)" ../$(EXE)


eiger:  $(EIGERSRC) $(EIGERINC) $(CXXSRC) $(CXXINC) Make.eiger
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CXXSRC) $(CXXINC) $(EIGERSRC) $(EIGERINC) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CXXSRC) $(EIGERSRC)" "INC = $(CXXINC) $(EIGERINC) " "EXE = ../$(EXE)" ../$(EXE)


f90csv:  $(CSVSRC) $(CSVINC) $(FSRC) $(FINC) Make.f90csv
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(FSRC) $(FINC) $(CSVSRC) $(CSVINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(FSRC) $(CSVSRC)" "INC = $(FINC) $(CSVINC) $(BINDGENFILES)" "EXE = ../$(FEXE)" ../$(FEXE)

f90: $(FSRC) $(FINC)  $(EIGERSRC) $(EIGERINC) Make.f90
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(FSRC) $(FINC) $(EIGERSRC) $(EIGERINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(FSRC) $(EIGERSRC)" "INC = $(FINC) $(EIGERINC) $(BINDGENFILES)" "EXE = ../$(FEXE)" ../$(FEXE)

f90fake:  $(EIGERSRC) $(EIGERINC) $(FSRC) $(FINC) Make.f90fake
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(FSRC) $(FINC) $(EIGERSRC) $(EIGERINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(FSRC) $(EIGERSRC)" "INC = $(FINC) $(EIGERINC) $(BINDGENFILES)" "EXE = ../$(FEXE)" ../$(FEXE)


cplain:  $(CSVSRC) $(CSVINC) $(CSRC) $(CINC) Make.cplain
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CSRC) $(CINC) $(CSVSRC) $(CSVINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CSRC) $(CSVSRC)" "INC = $(CINC) $(CSVINC) $(BINDGENFILES)" "EXE = ../$(CEXE)" ../$(CEXE)

fplain:  $(CSVSRC) $(CSVINC) $(FSRC) $(FINC) Make.fplain
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(FSRC) $(FINC) $(CSVSRC) $(CSVINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(FSRC) $(CSVSRC)" "INC = $(FINC) $(CSVINC) $(BINDGENFILES)" "EXE = ../$(FEXE)" ../$(FEXE)

ccsv:  $(CSVSRC) $(CSVINC) $(CSRC) $(CINC) Make.ccsv
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CSRC) $(CINC) $(CSVSRC) $(CSVINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CSRC) $(CSVSRC)" "INC = $(CINC) $(CSVINC) $(BINDGENFILES)" "EXE = ../$(CEXE)" ../$(CEXE)

c:  $(EIGERSRC) $(EIGERINC) $(CSRC) $(CINC) Make.c
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CSRC) $(CINC) $(EIGERSRC) $(EIGERINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CSRC) $(EIGERSRC)" "INC = $(CINC) $(EIGERINC) $(BINDGENFILES)" "EXE = ../$(CEXE)" ../$(CEXE)

cscreen:  $(EIGERSRC) $(EIGERINC) $(CSRC) $(CINC) Make.cscreen
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CSRC) $(CINC) $(EIGERSRC) $(EIGERINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CSRC) $(EIGERSRC)" "INC = $(CINC) $(EIGERINC) $(BINDGENFILES)" "EXE = ../$(CEXE)" ../$(CEXE)

cfake:  $(EIGERSRC) $(EIGERINC) $(CSRC) $(CINC) Make.cfake
	@if [ ! -d Obj_$@ ]; then mkdir Obj_$@; fi
	@cp -p $(CSRC) $(CINC) $(EIGERSRC) $(EIGERINC) $(BINDGENFILES) $(MAKESUPPORT) Obj_$@
	$(MAKE) genclean
	@cp Make.$@ Obj_$@/Makefile
	@cd Obj_$@; \
	$(MAKE) $(MAKEPLL) "SRC = $(CSRC) $(EIGERSRC)" "INC = $(CINC) $(EIGERINC) $(BINDGENFILES)" "EXE = ../$(CEXE)" ../$(CEXE)



$(CXXGENFILES): $(CXXSRC) updateLoggersCXX.sh
	./updateLoggersCXX.sh

$(FGENFILES): $(FSRC) updateLoggersF90.sh
	./updateLoggersF90.sh

$(CGENFILES): $(CSRC) updateLoggersC.sh
	./updateLoggersC.sh

# Clean

clean: clean_csv clean_eiger clean_fake clean_f90 clean_c clean_f90fake clean_cfake clean_f90csv clean_ccsv clean_cscreen clean_cplain clean_fplain clean_plain
	$(RM) *.o *.s *.mod

all: 
	$(MAKE) eiger
	$(RUNIT) cxxapp_eiger
	test -s tesla.cxxsub1_mpi_2.0.log
	rm tesla.cxxsub1_mpi_2.0.log 
	$(MAKE) c
	$(RUNIT) capp_c
	test -s tesla.csub1_mpi_2.0.log
	rm tesla.csub1_mpi_2.0.log
	$(MAKE) f90
	$(RUNIT) ./app_f90
	test -s tesla.sub1_mpi_2.0.log
	rm tesla.sub1_mpi_2.0.log 
	$(MAKE) csv
	$(RUNIT) cxxapp_csv
	test -s tesla.cxxsub1_mpi_2.0.log
	rm tesla.cxxsub1_mpi_2.0.log
	$(MAKE) ccsv
	$(RUNIT) capp_ccsv
	test -s tesla.csub1_mpi_2.0.log
	rm tesla.csub1_mpi_2.0.log
	$(MAKE) f90csv
	$(RUNIT) app_f90csv
	test -s tesla.sub1_mpi_2.0.log
	rm tesla.sub1_mpi_2.0.log
	$(MAKE) fake
	$(RUNIT) cxxapp_fake
	test -s tesla.cxxsub1_mpi_2.0.log
	test -s mpifakeeiger.1.log
	rm tesla.cxxsub1_mpi_2.0.log mpifakeeiger.1.log
	$(MAKE) cfake
	$(RUNIT) capp_cfake
	test -s mpifakeeiger.1.log
	test -s tesla.csub1_mpi_2.0.log 
	rm tesla.csub1_mpi_2.0.log mpifakeeiger.1.log
	$(MAKE) f90fake
	$(RUNIT) app_f90fake
	test -s tesla.sub1_mpi_2.0.log
	test -s mpifakeeiger.1.log
	rm tesla.sub1_mpi_2.0.log mpifakeeiger.1.log
	$(MAKE) cscreen
	$(RUNIT) capp_cscreen
	test -s tesla.csub1_mpi_2.0.log 
	rm tesla.csub1_mpi_2.0.log
	$(MAKE) plain
	$(RUNIT) cxxapp_plain
	test ! -f tesla.cxxsub1_mpi_2.0.log
	test ! -f mpifakeeiger.1.log
	$(MAKE) cplain
	$(RUNIT) capp_cplain
	test ! -f tesla.csub1_mpi_2.0.log
	test ! -f mpifakeeiger.1.log
	$(MAKE) fplain
	$(RUNIT) app_fplain
	test ! -f mpifakeeiger.1.log
	test ! -f tesla.sub1_mpi_2.0.log
	@echo end of tests

# all target combative over top dir generators. be careful at top
.NOTPARALLEL:

clean_cplain:
	rm -rf Obj_cplain capp_cplain
clean_fplain:
	rm -rf Obj_fplain app_fplain
clean_plain:
	rm -rf Obj_plain cxxapp_plain
clean_csv:
	rm -rf Obj_csv cxxapp_csv
clean_eiger:
	rm -rf Obj_eiger cxxapp_eiger
clean_fake:
	rm -rf Obj_fake cxxapp_fake
clean_f90:
	rm -rf Obj_f90 app_f90
clean_f90fake:
	rm -rf Obj_f90fake app_f90fake
clean_f90csv:
	rm -rf Obj_f90csv app_f90csv
clean_c:
	rm -rf Obj_c capp_c
clean_cscreen:
	rm -rf Obj_cscreen capp_cscreen
clean_cfake:
	rm -rf Obj_cfake capp_cfake
clean_ccsv:
	rm -rf Obj_ccsv capp_ccsv

distclean: clean genclean dumpclean dbclean
	(cd sand; $(MAKE) clean)

genclean:
	@$(RM) $(ALLGEN)

dumpclean:
	rm -rf dumps.*

dbclean:
	./resetdb
	$(RM) tesla.*.log mpifakeeiger*log
