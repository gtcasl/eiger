AM_CXXFLAGS = -std=gnu++0x

bin_PROGRAMS = eiger-loader 
eiger_loader_SOURCES = eiger_loader.cpp
eiger_loader_LDADD = libeiger.la 

lib_LTLIBRARIES = libeiger.la libfakeeiger.la
pkginclude_HEADERS = eiger.h fakekeywords.h
libeiger_la_SOURCES = eiger.cpp eiger.h default_backend.cpp sqlite3.c
libfakeeiger_la_SOURCES = eiger.cpp eiger.h fake_backend.cpp
libeiger_la_CPPFLAGS = -DSCHEMAFILE=\"$(pkgdatadir)/schema.sql\" -DSQLITE_OMIT_LOAD_EXTENSION $(PTHREAD_CFLAGS)
libeiger_la_LDFLAGS = $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)

pkgdata_DATA = ../database/schema.sql

ACLOCAL_AMFLAGS = -I m4
