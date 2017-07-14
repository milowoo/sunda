/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：系统管理工具--刷新某个服务
 *
 *  Edit History:
 *
 *    2012/04/06 - MiloWoo 建立.
 *************************************************************************/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include "sunda.h"

int main(int argc, char **argv)
{
    int rc, id;
    int ch;
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "%s.log", basename(argv[0]));

    ch = getopt(argc, argv, "i:");
    if (ch == -1 || ch == '?') 
    {
        printf("USAGE %s -i SRVID\n", argv[0]);
        return -1;
    }

    rc = ipc_connect();
    if (rc) {
        printf("mb_init error %d\n", rc);
        return -1;
    }

    id = atoi(optarg);
    printf("fresh server id = %d.....\n", id);

    rc = svr_cmd_deal(id, TSK_CMD_FRESH);
    if (rc) {
        printf("task exit error %d\n", rc);
        return -1;
    }
    
    printf("task fresh %d success  \n", id);

    return 0;
}
