include $(DEV_HOME)/mak/platform.mak
include $(DEV_HOME)/mak/database.mak
include $(DEV_HOME)/mak/tuxedo.mak

PRGOBJS = \
	commserver.o

PRGTARG = commserver
PRGLIBS = -ltux -lcommon -lpthread $(NETLIBS)
PRGDEFS =

# used for TUXEDO server
PRGSVCS =
PRGRSMN =

# used for db2 database
PRGDBNM =

debug all: debugexec
release: releaseexec

# DO NOT modify any code below!!!

releasedynamic debugdynamic releasestatic debugstatic releaseexec debugexec releasetuxsvrexec debugtuxsvrexec clean:
	@make -f $(DEV_HOME)/mak/mkstand.mak $@ TARGET="$(PRGTARG)" OBJS="$(PRGOBJS)" LIBS="$(PRGLIBS)" DEFS="$(PRGDEFS)" TUXSVCS="$(PRGSVCS)" TUXRSMN="$(PRGRSMN)" DBNM="$(PRGDBNM)"
