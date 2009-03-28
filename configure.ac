# Process this file with autoconf to produce a configure script.
# $Id: configure.ac,v 1.2 2009-03-28 03:59:07 nick Exp $
AC_INIT(README)

# Setup for automake
PROJ_NAME=froggix
PROJ_VERSION=1.0.0
AM_INIT_AUTOMAKE($PROJ_NAME, $PROJ_VERSION)

# Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Check for tools
AC_PROG_CC
AC_PROG_CXX

# Check for compiler environment
AC_C_CONST

# Figure out which math and OpenGL libraries to use
case "$target" in
    *-*-cygwin* | *-*-mingw32*)
        MATHLIB=""
        SYS_GL_LIBS="-lopengl32"
        ;;
    *-*-beos* | *-*-darwin*)
        MATHLIB=""
        SYS_GL_LIBS="-lGL"
        ;;
    *-*-aix*)
        if test x$ac_cv_prog_gcc = xyes; then
            CFLAGS="-mthreads"
        fi
        ;;
    *)
        MATHLIB="-lm"
        AC_PATH_X
        AC_PATH_XTRA
        if test x$have_x = xyes; then
            CFLAGS="$CFLAGS $X_CFLAGS"
            SYS_GL_LIBS="$X_LIBS -lGL"
        else
            SYS_GL_LIBS="-lGL"
        fi
        ;;
esac
AC_SUBST(MATHLIB)

# Check for SDL
SDL_VERSION=1.1.8
AM_PATH_SDL($SDL_VERSION, :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# Check for OpenGL
AC_MSG_CHECKING(for OpenGL support)
HAVE_OPENGL=no
AC_TRY_COMPILE([
 #include <GL/gl.h>
],[
],[
HAVE_OPENGL=yes
])
AC_MSG_RESULT($HAVE_OPENGL)
if test x$HAVE_OPENGL = xyes; then
    CFLAGS="$CFLAGS -DHAVE_OPENGL"
    GL_LIBS="$SYS_GL_LIBS"
else
    GL_LIBS=""
fi
AC_SUBST(GL_LIBS)

# Check for standard C++ library
AC_CHECK_LIB(stdc++, main)

# Finally create all the generated files
AC_OUTPUT(Makefile src/Makefile)
