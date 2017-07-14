/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：系统管理工具--清理消息队列以及FD链表
 *
 *  Edit History:
 *
 *    2012/04/09 - MiloWoo 建立.
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

static UINT64 get_curr_time()
{
	struct timeval now;
	uint64_t   utmp;
	uint64_t   currtime;
	
	gettimeofday(&now, NULL);
    utmp = now.tv_sec;
    currtime = utmp * 1000000 + now.tv_usec;
    return currtime;
}

int main(int argc, char **argv)
{
    int rc;
    int i, j;
    int iReqQueId;
    int iRspQueId;
    uint64_t   currtime;
	unsigned int diff_time = 0;
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "%s.log", basename(argv[0]));

    rc = ipc_connect();
    if (rc) {
        printf("mb_init error %d\n", rc);
        return -1;
    }

    iReqQueId = sdReqQueId();
    iRspQueId = sdRspQueId();
    
    printf("clear the req que ----\n", rc);
    MsqClear(iReqQueId);
    
    printf("clear the rsp que ----\n", rc);
    
    MsqClear(iRspQueId);
    
    currtime = get_curr_time();	
#if 0  
    printf("clear fd info ----\n", rc);
    
  
    for (i =0; i < MAX_CHLD_NUM; i++)
    {
    	if (GetCommPid(i) != -1)
    	{
    		for (j = 0; j < MAX_FD_NUM; j++)
    		{
    			if (GetCommClientFD(i,j) > 0)
    			{
    				diff_time = (currtime - GetCommClientTime(i, j))/1000000;
    				printf("i =[%d] j =[%d] fd  =[%d] diff [%ld] \n", i, j, GetCommClientFD(i,j), diff_time);
    			}
    		    else
    		    	break;
    		}
    	}
    	else
    	{
    		break;
    	}
    }
#endif
   
    printf("clear  success  \n");

    return 0;
}
