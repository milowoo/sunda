#|----------------------------------------------------------------------------|
#|                           TOPMake 2.0                                      |      |
#|    All Rights Reserved                                                     |
#|----------------------------------------------------------------------------|
#| FILE NAME    : sample.dynamiclib.mak                                       |
#| DESCRIPTIONS : Sample makefile for dynamic library                         |
#|----------------------------------------------------------------------------|

include $(DEV_HOME)/mak/platform.mak
include $(DEV_HOME)/mak/database.mak
include $(DEV_HOME)/mak/tuxedo.mak

PRGOBJS = \
	mxml-string.o \
	mxml-set.o \
	mxml-search.o \
	mxml-private.o \
	mxml-node.o \
	mxml-index.o \
	mxml-file.o \
	mxml-entity.o \
	mxml-attr.o \
	xml.o

PRGTARG = xml
PRGDEFS =

# used for db2 database
PRGDBNM =

debug all: debugstatic
release: releasestatic

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(DEV_HOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
