#|----------------------------------------------------------------------------|
#|               nline/src/pubfunc/mb
#|                           TOPMake 2.0                                      |
#| Copyright (c) 2000-2003 Shanghai Huateng Software Systems Co., Ltd.        |
#|    All Rights Reserved                                                     |
#|----------------------------------------------------------------------------|
#| FILE NAME    : sample.dynamiclib.mak                                       |
#| DESCRIPTIONS : Sample makefile for dynamic library                         |
#|----------------------------------------------------------------------------|

include $(DEV_HOME)/mak/platform.mak
include $(DEV_HOME)/mak/database.mak
include $(DEV_HOME)/mak/tuxedo.mak

PRGOBJS = \
	tuxcmd.o \
	tuxmgr.o \
	tuxmain.o

PRGTARG = tux
PRGDEFS =

# used for db2 database
PRGDBNM =

debug all: debugstatic
release: releasestatic

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(DEV_HOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
