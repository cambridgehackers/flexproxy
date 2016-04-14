# $Id: Makefile,v 1.7 2004/06/22 10:43:38 cvs Exp $

SHELL=/bin/sh
CC=cc
CXX=cc

CFLAGS=-g -DDEBUG
LDFLAGS=
CPPFLAGS=
CXXFLAGS=

SRC=flexproxy.c strutils.c conffile.c netfunc.c
OBJ=flexproxy.o strutils.o conffile.o netfunc.o

BIN=flexproxy

all: $(BIN)

flexproxy: $(OBJ)
	$(CC) -o flexproxy $(OBJ)

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(BIN)
