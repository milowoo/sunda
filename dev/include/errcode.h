#ifndef _ERR_CODE_H
#define _ERR_CODE_H

#define TUX_EXIT                  0   /* service failue with server exit */
#define TUX_FAIL                  1   /* service FAILure for tux_return */
#define TUX_SUCCESS               2   /* service SUCCESS for tux_return */

#define SYS_LIMIT               -2000
#define SYS_INVAL               -2001
#define SYS_OS                  -2002
#define SYS_NOENT               -2003
#define SYS_TIMEOUT             -2004
#define SYS_PROTO               -2005
#define SYS_SVCFAIL             -2006
#define SYS_EMATCH              -2007
#define SYS_GOTSIG              -2008
#define SYS_SYSTEM              -2009
#define SYS_ERR_SVC             -2010
#define SYS_DEAD_SVC            -2011

extern int          tux_errno;

#endif

