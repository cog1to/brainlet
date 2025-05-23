AC_INIT([brainlet],[0.1])
LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

dnl [Switch to a C++ compiler, and check if it works.]
AC_LANG(C++)
AC_PROG_CXX

dnl [Check for Qt6 libs]
PKG_CHECK_MODULES([QT], [Qt6Core Qt6Gui Qt6DBus Qt6Widgets Qt6Sql])

dnl [Get flags for Qt6]
CXXFLAGS="`$PKG_CONFIG --cflags-only-I Qt6Core Qt6Gui Qt6DBus Qt6Widgets Qt6Sql` $CXXFLAGS"
LDFLAGS="`$PKG_CONFIG --libs-only-L Qt6Core Qt6Gui Qt6DBus Qt6Widgets Qt6Sql` $LDFLAGS"
LIBS="`$PKG_CONFIG --libs-only-l Qt6Core Qt6Gui Qt6DBus Qt6Widgets Qt6Sql` $LIBS"

dnl [Check for moc and rcc]
QTEXEC="`$PKG_CONFIG --variable=libexecdir Qt6Core`"
AC_PATH_PROG(RCC, [rcc], [], ["$QTEXEC"])
AC_PATH_PROG(MOC, [moc], [], ["$QTEXEC"])

dnl [Process Makefile.in to create Makefile]
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

