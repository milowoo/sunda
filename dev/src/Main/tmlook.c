/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：系统管理工具--察看系统信息
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
#include "IpcOpr.h"
#include "CfgOpr.h"
#include "trace.h"

int main(int argc, char *argv[])
{
    char flag[2 + 1];
    int rc;
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "%s.log", basename(argv[0]));

    memset(flag, 0, sizeof(flag));
    if (argc > 1 && strlen(argv[1]) == 2) 
    {
        memcpy(flag, argv[1], 2);
    }

    rc = ipc_connect();
    if (rc) 
    {
        printf("mb_connect error: %d\n", rc);
        exit(1);
    }
   
    list_info(flag);

    return 0;
}

int list_info(char *flag)
{
   if (!memcmp(flag, "-r", 2)) 
   {
       list_server();
   } 
   else if (!memcmp(flag, "-s", 2)) 
   {
       list_service();
   }
   
   else if (!memcmp(flag, "-q", 2)) 
   {
       list_queue();
   }
   
   
    
   return 0;
}
