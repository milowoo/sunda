include $(DEV_HOME)/mak/platform.mak
include $(DEV_HOME)/mak/database.mak
include $(DEV_HOME)/mak/tuxedo.mak

PRGOBJS = \
	simu.o

PRGTARG = simu
PRGLIBS = -lcommon
PRGDEFS =

# used for db2 database
PRGDBNM =

debug all: debugexec
release: releaseexec

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec clean:
	@make -f $(DEV_HOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" DBNM="$(PRGDBNM)"
