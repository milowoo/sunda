#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sunda.h"
#include "trace.h"


int giTimeOut; 

static void time_out(int i)
{
	SysLog(SYS_TRACE_ERROR, "over time!");
	giTimeOut = 1;
}

int main(int argc , char ** argv)
{
	int i =0;
	int j =0;
	int ssn = 0;
	int iRet; 
	int sock_id;
	int ret_tot=0, icount = 0;
	int port = 61107;
	int iLen = 0;
	char sIpAdd[25+1]={0};
	char sBuf[256+1]={0};
	char sReqBuf[256+1]={0};
	char sRspBuf[256+1]={0};
	sunda_req_head_t req_head={0};
	
	giTimeOut = 0;
	
	signal(SIGALRM,  time_out);
	
	if (argc >= 2)
	{
		i = atoi(argv[1]);
	}
	
	j = i %10;
	sleep(1);
	
	memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "simu_%d.log", j);
	
	strcpy(sIpAdd, "192.168.44.3");
	
	for (i = 0 ; i < 20; i++)
	{
		giTimeOut  = 0;
		iRet = OpenSocket(sIpAdd, port, 99, 1, &sock_id);
	    if (iRet != 0)
	    {
	    	SysLog(SYS_TRACE_ERROR,"OpenSocket port %d error %d %s\n", port, errno, strerror(errno));
	    	sleep(1);
	    	continue;
            // exit(2);
	    }
	    
	    memset(&req_head, 0, sizeof(req_head));
	    
	    memcpy(req_head.name, "testsvr", 7);
	    req_head.ssn = (time(NULL) + ssn);
	    req_head.err_no = 0;
	    ssn++;
	    
	    memset(sBuf, 0, sizeof(sBuf));
	    memcpy(sBuf, (char *)&req_head, sizeof(req_head));
	    memcpy(sBuf+sizeof(req_head), "test111111", 10);
	    iLen = sizeof(req_head) + 10;
	    sprintf(sReqBuf, "%04d", iLen);
	    memcpy(sReqBuf+4, sBuf, iLen);

	    alarm(2);
	    iRet = WriteSocket(sock_id, sReqBuf, iLen+4);
	    alarm(0);
	    ret_tot++;
	    if (iRet != 0)
	    {
	    	if (giTimeOut == 1)
	    	{
	    		SysLog(SYS_TRACE_ERROR," time over ");
	    	}
	    	
	    	close(sock_id);
	    	SysLog(SYS_TRACE_ERROR,"WriteSocket error %d %s\n",errno, strerror(errno));
	    	sleep(1);
            continue;
	    }
	    
	    giTimeOut  = 0;
	    
	    memset(sBuf, 0, sizeof(sBuf));
	    alarm(3);
	    iRet = RcvTxnMsg(sock_id, 4, sBuf, &iLen);
	    alarm(0);
	    if (iRet != 0)
	    {
	    	if (giTimeOut == 1)
	    	{
	    		SysLog(SYS_TRACE_ERROR," time over ");
	    	}
	    	close(sock_id);
	    	SysLog(SYS_TRACE_ERROR,"RcvTxnMsg  error %d %s\n", errno, strerror(errno));
            exit(2);
	    }
	    
	    close(sock_id);
	    
	    icount++;
	    
	    memset(&req_head, 0, sizeof(req_head));
	    memcpy((char*)&req_head, sBuf+4, sizeof(req_head));
	    memcpy(sRspBuf, sBuf+(4+sizeof(req_head)), iLen-sizeof(req_head)-4); 
	    
	    if (req_head.err_no != 0)
	       SysLog(SYS_TRACE_ERROR,"ssn [%d] err ", ssn-1);

/*	       
	    else 
	    	SysLog(SYS_TRACE_ERROR,"sRspBuf [%s]", sRspBuf);
*/
	}
	
	SysLog(SYS_TRACE_ERROR,"ret_tot [%d] tot respone  [%d]", ret_tot, icount);
		
    return 0;
}

