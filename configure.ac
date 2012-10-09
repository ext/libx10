AC_INIT([libx10],[0.1.0], [ext@sidvind.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

AC_CONFIG_FILES([
	Makefile
])

AC_PROG_CC_C99
AC_C_BIGENDIAN

pkgdatadir="${datadir}/${PACKAGE}"
AC_SUBST(pkgdatadir)
erlanglibdir="${pkgdatadir}/erlang"
AC_SUBST(erlanglibdir)

ac_sys_system=`uname -s`

case $ac_sys_system in
	Darwin*)
		DARWIN_CFLAGS="-arch ppc -arch ppc64 -arch i386 -arch x86_64"
		;;
esac

AC_SUBST(DARWIN_CFLAGS)

AC_ARG_ENABLE([erlang],
	[AS_HELP_STRING([--enable-erlang=PATH], [bindings for erlang, set PATH to the Erlang include directory])],
	[
		AM_CONDITIONAL(X10_BUILD_ERLANG, true)
		if test x${enableval} = xyes; then
			ERLANG_CFLAGS="-I${libdir}/erlang/usr/include"
			ERLANG_CFLAGS="$ERLANG_CFLAGS -I/usr/lib/erlang/usr/include"
			ERLANG_CFLAGS="$ERLANG_CFLAGS -I/usr/local/lib/erlang/usr/include"
			ERLANG_CFLAGS="$ERLANG_CFLAGS -I/opt/local/lib/erlang/usr/include"	    
		else
			ERLANG_CFLAGS="-I$enableval"
		fi
		
		OLD_CFLAGS="$CFLAGS"
		OLD_CPPFLAGS="$CPPFLAGS"
 		CFLAGS="$ERLANG_CFLAGS $CFLAGS"
		CPPFLAGS="$ERLANG_CFLAGS $CPPFLAGS"
		
		AM_CONDITIONAL(X10_BUILD_ERLANG, true)
		AC_PATH_PROG([ERL], [erl])
		AC_PATH_PROG([ERLC], [erlc])
		
		AC_CHECK_HEADERS([erl_driver.h], [],
		[
			AC_MSG_ERROR([Could not find the `erl_driver.h' header. Make sure to append the correct include path with --enable-erlang=PATH])
		])
			
		CFLAGS="$OLD_CFLAGS"
		CPPFLAGS="$OLD_CPPFLAGS"
	],
	[
		AM_CONDITIONAL(X10_BUILD_ERLANG, false)
	])

AC_SUBST(ERLANG_CFLAGS)

AC_OUTPUT
