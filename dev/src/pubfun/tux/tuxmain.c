/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能: 系统服务接入框架入口
 *
 *  Edit History:
 *
 *    2012/03/16 - MiloWoo 建立.
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <setjmp.h>
#include "sunda.h"

static void exit_handler(int sig)
{
    SysLog(SYS_TRACE_NORMAL,"receive SIGTERM signal, exit!");
    tux_svrdone();
    exit(0);
}

int main(int argc, char **argv)
{
    int rc = 0;
    int svrid, svc_pos;
    char sLogFileName[128 + 1]={0};

    memset(gsLogFile, 0, sizeof(gsLogFile));
    strcpy(gsLogFile, "ULOG");

    if (argc < 4) 
    {
        SysLog(SYS_TRACE_ERROR,"usage %s pid pcsid", basename(argv[0]));
        return -1;
    }

    if (atol(argv[1]) != getpid()) 
    {
        SysLog(SYS_TRACE_ERROR,"start process illegal, %ld %ld", atol(argv[1]), getpid());
        return -1;
    }

    svc_pos = atoi(argv[2]);
    svrid = atoi(argv[3]);

    sigset(SIGHUP,  SIG_IGN);
    sigset(SIGINT,  SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    sigset(SIGTERM, exit_handler);

    rc = ipc_connect();
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"ipc_connect error %d", tux_errno);
        return rc;
    }
    
    SysLog(SYS_TRACE_NORMAL,"SERVER %d STAR SUCCSS ", svrid);
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "%s.%d.log", (char *)basename(argv[0]), sdGetSvrPidSeq(atoi(argv[2])));

    rc = tux_svrinit(argc, argv);
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"sys_svrinit error %d", rc);
        tux_svrdone();
        return rc;
    }

    while (1) 
    {
        rc = tux_service(svrid, svc_pos);
        if (rc) 
        {
            SysLog(SYS_TRACE_ERROR,"mb_service error %d", tux_errno);
            if (tux_errno != SYS_NOENT)
                break;
        }
    }

    tux_svrdone();
    return -1;
}
