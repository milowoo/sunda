/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：系统管理工具--停止系统
 *
 *  Edit History:
 *
 *    2012/03/16 - MiloWoo 建立.
 *************************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include "sunda.h"
#include "CfgOpr.h"
#include "trace.h"

int main(int argc, char *argv[])
{
    int rc;
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "%s.log", argv[0]);

    rc = ipc_connect();
    if (rc) {
        fprintf(stderr, "ipc_connect error %d\n", rc);
        SysLog(SYS_TRACE_ERROR,"ipc_connect err ");
        return -1;
    }

    rc = ipc_remove();
    if (rc) 
    {
        printf("ipc_remove error: %d\n", rc);
        SysLog(SYS_TRACE_ERROR,"ipc_remove err ");
        return -2;
    }
    
    printf(" down system success \n");

    return 0;
}
