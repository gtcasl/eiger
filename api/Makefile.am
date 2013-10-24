bin_PROGRAMS = eiger-loader eiger-mpi-loader
eiger_loader_SOURCES = feloader.cpp
eiger_mpi_loader_SOURCES = mfeloader.cpp
eiger_loader_LDADD = libeigerInterface.la $(MYSQLPP_LIBS)
eiger_mpi_loader_LDADD = libeigerInterface.la $(MYSQLPP_LIBS)

lib_LTLIBRARIES = libeigerInterface.la libfakeeigerInterface.la libmpifakeeigerInterface.la
pkginclude_HEADERS = eiger.h fakeeiger.h mpifakeeiger.h fakekeywords.h
libeigerInterface_la_SOURCES = eiger.cpp eiger.h
libfakeeigerInterface_la_SOURCES = fakeeiger.cpp fakeeiger.h
libmpifakeeigerInterface_la_SOURCES = mpifakeeiger.cpp mpifakeeiger.h fakekeywords.h
libeigerInterface_la_CPPFLAGS = $(MYSQLPP_CFLAGS) $(MYSQL_CFLAGS)
libeigerInterface_la_LIBADD = $(MYSQLPP_LIBS) $(MYSQL_LIBS)

ACLOCAL_AMFLAGS = -I m4
