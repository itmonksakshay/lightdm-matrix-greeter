AC_INIT([lightdm-matrix-greeter],[1.0.0],[akshay.bhatt@hotmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AM_PROG_CC_C_O

PKG_CHECK_MODULES([GTK], [gtk+-3.0])
PKG_CHECK_MODULES([GMODULE], [gmodule-export-2.0])
PKG_CHECK_MODULES([LIGHTDMGOBJECT], [liblightdm-gobject-1 >= 1.19.2])

AC_CHECK_PROG([HAVE_EXO_CSOURCE], [exo-csource], [yes])
if test x"$HAVE_EXO_CSOURCE" != xyes; then
  AC_MSG_FAILURE([exo-csource not installed])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])
AC_OUTPUT
