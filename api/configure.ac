AC_INIT([eiger], [1.0], [eanger@gatech.edu])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR

AX_PROG_CXX_MPI

LT_INIT

# mysql
AC_ARG_WITH([mysql-include-path],
			[AS_HELP_STRING([--with-mysql-include-path],
							[location of the MySQL headers, defaults to /usr/include/mysql])],
			[MYSQL_CFLAGS="-I$withval"],
			[MYSQL_CFLAGS='-I/usr/include/mysql'])
AC_SUBST([MYSQL_CFLAGS])

AC_ARG_WITH([mysql-lib-path],
			[AS_HELP_STRING([--with-mysql-lib-path],
							[location of the MySQL libraries])],
			[MYSQL_LIBS="-L$withval -lmysqlclient"],
			[MYSQL_LIBS='-lmysqlclient'])
AC_SUBST([MYSQL_LIBS])

# mysql++
AC_ARG_WITH([mysqlpp-include-path],
			[AS_HELP_STRING([--with-mysqlpp-include-path],
							[location of the MySQL++ headers, defaults to /usr/include/mysql++])],
			[MYSQLPP_CFLAGS="-I$withval"],
			[MYSQLPP_CFLAGS='-I/usr/include/mysql++'])
AC_SUBST([MYSQLPP_CFLAGS])

AC_ARG_WITH([mysqlpp-lib-path],
			[AS_HELP_STRING([--with-mysqlpp-lib-path],
							[location of the MySQL++ libraries])],
			[MYSQLPP_LIBS="-L$withval -lmysqlpp"],
			[MYSQLPP_LIBS='-lmysqlpp'])
AC_SUBST([MYSQLPP_LIBS])


AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
