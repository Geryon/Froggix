# $Id: Makefile.am,v 1.2 2009-03-28 03:52:29 nick Exp $

DISTCLEANFILES = *~ *.~* *.exe

AM_CFLAGS = -Wall `sdl-config --cflags`
AM_LDFLAGS = `sdl-config --libs` -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf

bin_PROGRAMS = froggix
froggix_SOURCES = froggix.c
