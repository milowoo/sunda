
SHELL = /bin/sh

CC    = gcc
CXX   = GCC
DLINK = ld
SLINK = ar
DLFLG = -G
SLFLG = ruc
#CCFLG = -Xa -misalign
#CXXFLG= -misalign -KPIC
#EXFLG = -Xa -misalign
CXXFLG= -Xa -KPIC
DBFLG = -g
OPFLG = -O
DLFIX = .so
SLFIX = .a
#UXLIBS = -lmalloc -lm -lc
CPPLIBS = -lstdc++
DLLIBS = -ldl
CURLIBS = -lcurses
CRYLIBS =
THREADLIBS = -lrt -lpthread
XLIBS   = -lXext -lX11
XLIBP   = /usr/openwin/lib
OSDEF = -DSUNOS_UNIX -D_REENTRANT
OSTYPE = aix
JAVAMD = aix
