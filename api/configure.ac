AC_INIT([eiger], [3.0], [eanger@gatech.edu])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR

AC_PROG_CXX

LT_INIT

# mysql
AC_ARG_WITH([sqlite-include-path],
			[AS_HELP_STRING([--with-sqlite-include-path],
							[location of the sqlite3 headers, defaults to /usr/include])],
			[SQLITE_CFLAGS="-I$withval"],
			[SQLITE_CFLAGS='-I/usr/include'])
AC_SUBST([SQLITE_CFLAGS])

AC_ARG_WITH([sqlite-lib-path],
			[AS_HELP_STRING([--with-sqlite-lib-path],
							[location of the sqlite3 library])],
			[SQLITE_LIBS="-L$withval -lsqlite3"],
			[SQLITE_LIBS='-lsqlite3'])
AC_SUBST([SQLITE_LIBS])

AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
