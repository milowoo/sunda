/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：短链接同步堵塞通讯服务
 *
 *  Edit History:
 *
 *    2012/04/04 - MiloWoo 建立.
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
#include <sys/socket.h> /* for socketpair */
#include <netinet/in.h>
#include "sunda.h"

#define SOCKET_MSG_HEAD_LEN    4
#define MAX_CHILD_NUM          5000

typedef struct
{
    pid_t  pid;
    int    socket_id;
}gpidSonPidDef;

gpidSonPidDef  gpidSonPid[MAX_CHILD_NUM];

static void main_exit(int sig);

static int  giTimeOut;
static int  giLocPort;
static char gsCltIP[25];
static int  giRmtPort;
static int  gpidtMainPid;
static int  giLstnScktId;
static int  giConnScktId;
static int  giClntScktId;
static int  nChdNum;
static int  gSvrId;



struct  sockaddr_in Client;

void time_out(int i)
{
	SysLog(SYS_TRACE_ERROR, "over time!");
	giTimeOut = 1;
}



static void child_exit(int sig)
{
    int   i, Status, iRet;
    pid_t nPid;

    while (1) 
    {
        nPid = waitpid((pid_t)(-1), &Status, WNOHANG|WUNTRACED);
        if (nPid <= 0) break;
        SysLog(SYS_TRACE_ERROR,"[PARENT]: child process %ld exit", nPid);
        for (i = 0; i < MAX_CHILD_NUM; i++) 
        {
            if (gpidSonPid[i].pid == nPid)
            {
            	gpidSonPid[i].pid = -1;
            	gpidSonPid[i].socket_id = -1;
            	return ;
            }
        }

        if (i == MAX_CHILD_NUM)
            return ;
    }
    
    return ;

}

static void main_exit(int sig)
{
    int i, iRet;

    sigset(SIGCLD, SIG_IGN);

    SysLog(SYS_TRACE_ERROR,"[PARENT]: kill all child process");

    for (i = 0; i < MAX_CHILD_NUM; i++) 
    {
        if (gpidSonPid[i].pid > 0) 
        {
            iRet = kill(gpidSonPid[i].pid, SIGTERM);
            if (iRet) 
            {
                 SysLog(SYS_TRACE_ERROR,"[PARENT]: kill child process %ld error, errno is %d", gpidSonPid[i].pid, errno);
            }
            
            CloseSocket(gpidSonPid[i].socket_id);
        }
    }

    sleep(2);

    CloseSocket(giLstnScktId);

    exit(1);
}

/**********************************************************
*函数名称: comm_init
*函数功能: 服务初始化
*输入参数: 无
*输出参数: 无
*返回参数: 无
***********************************************************/
int comm_init(int argc , char **argv)
{
	int iRet, i, ch;
	
	memset(gsLogFile, 0, sizeof(gsLogFile));
    strcpy(gsLogFile, "ULOG");

    if (argc < 4) 
    {
        SysLog(SYS_TRACE_ERROR,"usage %s pid pid_sen svr_id", basename(argv[0]));
        return -1;
    }

    if (atol(argv[1]) != getpid()) 
    {
        SysLog(SYS_TRACE_ERROR,"start process illegal, %ld %ld", atol(argv[1]), getpid());
        return -1;
    }
    
    gSvrId = atoi(argv[3]);

    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "comm1_%d.log", gSvrId);
	
	while ((ch = getopt(argc-3, argv+3, "p:")) != -1) 
	{
        switch (ch) 
        {
        case 'p':
        	SysLog(SYS_TRACE_NORMAL, "p  optarg %s ", optarg);
            giLocPort = atoi(optarg);
            SysLog(SYS_TRACE_NORMAL, " server listen giLocPort %d ", giLocPort);
            break;
        }
    }
    
    iRet = ipc_connect();
    if (iRet) 
    {
        SysLog(SYS_TRACE_ERROR,"sys_init error");
        return -1;
    }
    
    nChdNum = 0;
    
    return 0;
}

void pack_err_rsp(char *sDestBuf, int *iRspLen, sunda_req_head_t *req_head)
{
	req_head->err_no = 99;
	
	sprintf(sDestBuf, "%0*d", SOCKET_MSG_HEAD_LEN, sizeof(sunda_req_head_t));
	memcpy(sDestBuf+SOCKET_MSG_HEAD_LEN, (char *)req_head, sizeof(sunda_req_head_t));
	*iRspLen = sizeof(sunda_req_head_t) + SOCKET_MSG_HEAD_LEN;
	
	return ;
}

int call_svr(char *sSrcBuf, int iBufLen, char *sDestBuf, int *iRspLen)
{
	int iRet; 
	int  iLen;
	long lReqLen = 0, lRspLen = 0;
	char sSvcName[TUX_NAME_LEN+1]={0};
	char sReqBuf[10240+1]={0};
	char sRspBuf[10240+1]={0};
    sunda_req_head_t req_head={0};
	pid_t pid = getpid();
	
	memcpy((char *)&req_head, sSrcBuf+SOCKET_MSG_HEAD_LEN, sizeof(req_head));
    memcpy(sSvcName, req_head.name, TUX_NAME_LEN);
    AllTrim(sSvcName);
	
	memset(&gipcMsgHead, 0, sizeof(gipcMsgHead));
    strcpy(gipcMsgHead.name, sSvcName);
    gipcMsgHead.req_msg_type = pid;
    gipcMsgHead.fd  = 0; 
    gipcMsgHead.ssn = req_head.ssn;
    gipcMsgHead.utime  = time(NULL);
    gipcMsgHead.reply_flag = REPLY_FLAG_ASYN;
    gipcMsgHead.len = iBufLen -  SOCKET_MSG_HEAD_LEN - sizeof(req_head);
    
    memset(sReqBuf, 0, sizeof(sReqBuf));
    memcpy(sReqBuf, (char *)&gipcMsgHead, sizeof(gipcMsgHead));
    memcpy(sReqBuf+sizeof(gipcMsgHead),  sSrcBuf+(SOCKET_MSG_HEAD_LEN + sizeof(req_head)), gipcMsgHead.len);
    lReqLen = gipcMsgHead.len+sizeof(gipcMsgHead);  
    iRet = tux_call(sSvcName, sReqBuf, lReqLen, sRspBuf, &lRspLen, 0);
    if (iRet != 0)
    {
    	pack_err_rsp(sDestBuf, iRspLen, &req_head);
    	SysLog(SYS_TRACE_ERROR , "sd_acall err %d !", tux_errno);
    	return -1;
    }
    
    req_head.err_no = 0;   
    iLen = lRspLen - sizeof(gipcMsgHead) + sizeof(req_head);
    sprintf(sDestBuf, "%0*d", SOCKET_MSG_HEAD_LEN, iLen);
    memcpy(sDestBuf+SOCKET_MSG_HEAD_LEN, (char *)&req_head, sizeof(req_head));
    memcpy(sDestBuf+(sizeof(req_head)+SOCKET_MSG_HEAD_LEN), sRspBuf+sizeof(gipcMsgHead), lRspLen - sizeof(gipcMsgHead));
    *iRspLen = iLen + SOCKET_MSG_HEAD_LEN;
    
    return 0;
}

/**********************************************************
*函数名称: child_proc
*函数功能: 子进程进行报文转发
*输入参数: iChdNum -- 进程编号
*输出参数: 无
*返回参数: 无
***********************************************************/
int child_proc(int sock_id)
{
    int  iRet ;
    int  iBufLen = 0;
    int  iRspLen;
    char sSrcBuf[8096+1]={0};
    char sDestBuf[8096+1]={0};
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "comm_child_%d.log", gSvrId);
    
    signal(SIGALRM,  time_out);
    memset(sSrcBuf, 0, sizeof(sSrcBuf));

    /* 取交易报文  */
    iRet = RcvTxnMsg(sock_id, SOCKET_MSG_HEAD_LEN, sSrcBuf, &iBufLen);
    if (iRet !=  0)
    {
        SysLog(SYS_TRACE_ERROR, "RcvTxnMsg err [%d] ", iRet);
        return -1;
    }

    iRet = call_svr(sSrcBuf, iBufLen, sDestBuf, &iRspLen);
    if (iRet != 0)
    {
    	SysLog(SYS_TRACE_ERROR, "call_svr err [%d] ", iRet);
    	
        return -1;
    }
    
    /* 发送报文至目标系统 */
    alarm(1);
    iRet = WriteSocket(sock_id, sDestBuf, iRspLen);
    alarm(0);
    if (iRet < 0)
    {
      
        SysLog(SYS_TRACE_ERROR , "WriteSocket iRet[%d]" , iRet );
        return -1;
    }
    
    SysLog(SYS_TRACE_ERROR , "WriteSocket ok !");
    sleep(1);
    return 0;
}

void reg_child(int nPid, int socket_id)
{
	int i = 0;
	
	for (i = 0; i < MAX_CHILD_NUM; i++)
	{
		if (gpidSonPid[i].pid == -1)
		{
			gpidSonPid[i].pid = nPid;
			gpidSonPid[i].socket_id = socket_id;
		}
	}
	
	return ;
}

int main(int argc, char **argv)
{
    int    iRet, i;
    int    nPid;
    
    /* 进程初始化 */
    iRet = comm_init(argc, argv);
    if (iRet != 0)
    {
        SysLog(SYS_TRACE_ERROR,  "comm_init  err ");
        exit(1);
    }
    
    /* 创建本地服务 */
    iRet = CreateSocket(giLocPort, 9999, 10, 0,&giLstnScktId);
    if (iRet != 0)
    {
        SysLog(SYS_TRACE_ERROR, "CreateSocket error iRet [%d]", iRet);
        exit(-1);
    }
    
    /*设置信号处理*/
    sigset(SIGCLD,  child_exit);
    sigset(SIGTERM, main_exit);
    
    SysLog(SYS_TRACE_NORMAL , " -------- port %d 等待客户端连接 ----------- ", giLocPort);
       
    while (1)
    {
        char sRmtCltAddr[25+1]={0};
        memset(&Client, 0, sizeof(Client));
        iRet = AcceptSocket(giLstnScktId , &Client, 5, &giConnScktId);
        if (iRet != 0)
        {
            close(giConnScktId);
            SysLog(SYS_TRACE_ERROR, "AcceptSocket 错误!" );
            continue;
        }
        
        /* 校验远端客户端地址　*/
        memset(sRmtCltAddr, 0, sizeof(sRmtCltAddr));
        strcpy(sRmtCltAddr, (char *)inet_ntoa(Client.sin_addr));
        SysLog(SYS_TRACE_ERROR,  "Client IP: [%s], nSocket: [%d]", sRmtCltAddr, giConnScktId);  
        nPid = fork();
        if (nPid < 0) 
        {
            SysLog(SYS_TRACE_ERROR,  "fork error: [%s]", strerror(errno));
            close(giConnScktId);
            continue;
        }
        
        if(nPid == 0) 
        {
            child_proc(giConnScktId);
            close(giConnScktId);
			exit(0);
        }
        
        nChdNum++;        
        reg_child(nPid, giConnScktId);       
        close(giConnScktId);
    }

    return 0;
}

/******************************************* 文件结束 **********************************************/

