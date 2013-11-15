bin_PROGRAMS = eiger-loader 
eiger_loader_SOURCES = eiger_loader.cpp
eiger_loader_CPPFLAGS = $(MYSQLPP_CFLAGS) $(MYSQL_CFLAGS)
eiger_loader_LDADD = libeigerInterface.la $(MYSQLPP_LIBS)

lib_LTLIBRARIES = libeigerInterface.la libfakeeigerInterface.la 
pkginclude_HEADERS = eiger.h fakekeywords.h
libeigerInterface_la_SOURCES = eiger.cpp eiger.h
libfakeeigerInterface_la_SOURCES = fakeeiger.cpp eiger.h
libeigerInterface_la_CPPFLAGS = $(MYSQLPP_CFLAGS) $(MYSQL_CFLAGS)
libeigerInterface_la_LIBADD = $(MYSQLPP_LIBS) $(MYSQL_LIBS)

ACLOCAL_AMFLAGS = -I m4
