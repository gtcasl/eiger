AC_INIT([eiger], [3.0], [eanger@gatech.edu])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR]) dnl workaround for automake 1.11

AC_PROG_CXX

LT_INIT

AX_PTHREAD

AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
