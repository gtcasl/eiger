AM_CXXFLAGS = -std=gnu++0x

bin_PROGRAMS = eiger-loader 
eiger_loader_SOURCES = eiger_loader.cpp
eiger_loader_CPPFLAGS = $(MYSQLPP_CFLAGS) $(MYSQL_CFLAGS)
eiger_loader_LDADD = libeiger.la $(MYSQLPP_LIBS) $(MYSQL_LIBS)

lib_LTLIBRARIES = libeiger.la libfakeeiger.la
pkginclude_HEADERS = eiger.h fakekeywords.h
libeiger_la_SOURCES = eiger.cpp eiger.h default_backend.cpp
libfakeeiger_la_SOURCES = eiger.cpp eiger.h fake_backend.cpp
libeiger_la_CPPFLAGS = $(SQLITE_CFLAGS) -DSCHEMAFILE=\"$(pkgdatadir)/schema.sql\"
libeiger_la_LIBADD = $(SQLITE_LIBS)

pkgdata_DATA = ../database/schema.sql

ACLOCAL_AMFLAGS = -I m4
