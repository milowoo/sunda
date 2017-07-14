include $(DEV_HOME)/mak/platform.mak
include $(DEV_HOME)/mak/database.mak
include $(DEV_HOME)/mak/tuxedo.mak

PRGOBJS = \
	funstr.o \
	funtime.o \
	funfile.o \
	funtrace.o \
	funsock.o \
	funmem.o \
	funsem.o \
	funmsq.o

PRGTARG = common
PRGDEFS =

# used for db2 database
PRGDBNM =

debug all: debugstatic
release: releasestatic

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(DEV_HOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
