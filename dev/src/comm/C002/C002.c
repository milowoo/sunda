/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：长链接通讯服务
 *
 *  Edit History:
 *
 *    2012/03/18 - MiloWoo 建立.
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
#include <sys/socket.h> 
#include <netinet/in.h>
#include "sunda.h"

typedef struct
{
    pid_t  pid;
    int    seq;
}gpidSonPidDef;

gpidSonPidDef  gpidSonPid[2];

static void main_exit(int sig);

static int  giTimeOut;
static int  giLocPort;
static char gsCltIP[25];
static int  giRmtPort;
static int  gpidtMainPid;
static int  giLstnScktId;
static int  giConnScktId;
static int  giClntScktId;
static int  gSvrId;

#define SOCKET_MSG_HEAD_LEN    4

struct  sockaddr_in Client;

void TimeOut(int i)
{
	SysLog(SYS_TRACE_ERROR, "over time!");
	giTimeOut = 1;
}

void clt_child_kill(int i)
{
	SysLog(SYS_TRACE_ERROR, "clt_child_kill ");
	close(giClntScktId);
}

void svr_child_kill(int i)
{
	SysLog(SYS_TRACE_ERROR, "clt_child_kill ");
	close(giConnScktId);
}

static void child_exit(int sig)
{
    int   i, Status, iRet;
    pid_t nPid;

    sighold(SIGTERM);

    while (1) 
    {
        nPid = waitpid((pid_t)(-1), &Status, WNOHANG|WUNTRACED);
        if (nPid <= 0) break;
        SysLog(SYS_TRACE_ERROR,"[PARENT]: child process %ld exit", nPid);
        for (i = 0; i < 2; i++) 
        {
            if (gpidSonPid[i].pid == nPid)
                break;
        }

        if (i == 2)
            continue;
            
        if ((gpidtMainPid = fork()) == -1)
        {
            SysLog(SYS_TRACE_ERROR, "Call fork() err err [%d ][%s]", errno, strerror(errno));
            main_exit(1);
            exit(-1);
        }

        if (gpidtMainPid == 0)
        {
            close(giLstnScktId);
            ChdMainProc(i+1);
            exit(0);
        }
        
        gpidSonPid[i].pid  = gpidtMainPid;
        gpidSonPid[i].seq  = i+1;
    }

    sigrelse(SIGTERM);
}

static void main_exit(int sig)
{
    int i, iRet;

    sigset(SIGCLD, SIG_IGN);

    SysLog(SYS_TRACE_ERROR,"[PARENT]: kill all child process");

    for (i = 0; i < 2; i++) 
    {
        if (gpidSonPid[i].pid > 0) 
        {
            iRet = kill(gpidSonPid[i].pid, SIGTERM);
            if (iRet) 
            {
                 SysLog(SYS_TRACE_ERROR,"[PARENT]: kill child process %ld error, errno is %d", gpidSonPid[i].pid, errno);
            }
        }
    }

    sleep(5);

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
    sprintf(gsLogFile, "long_comm_%d.log", gSvrId);
	
	while ((ch = getopt(argc-3, argv+3, "p:i:o:")) != -1) 
	{
        switch (ch) 
        {
        case 'p':
        	SysLog(SYS_TRACE_NORMAL, "p  optarg %s ", optarg);
            giLocPort = atoi(optarg);
            SysLog(SYS_TRACE_NORMAL, " server listen giLocPort %d ", giLocPort);
            break;
        case 'i':
        	SysLog(SYS_TRACE_NORMAL, "i optarg %s ", optarg);
            strncpy(gsCltIP, optarg, 12);
            SysLog(SYS_TRACE_NORMAL, "connect remote ip %s\n", gsCltIP);
            break;
        case 'o':
        	SysLog(SYS_TRACE_NORMAL, "o optarg %s ", optarg);
            giRmtPort = atoi(optarg);
            SysLog(SYS_TRACE_NORMAL, "connect remote giRmtPort %d\n", giRmtPort);
            break;
        }
    }
    
    for (i = 0; i < 2; i++)
    {
        gpidSonPid[i].pid = -1;
    }
    
    iRet = ipc_connect();
    if (iRet) 
    {
        SysLog(SYS_TRACE_ERROR,"sys_init error");
        return -1;
    }
    
    return 0;
}


/**********************************************************
*函数名称: ChdDoSvr
*函数功能: 子进程当服务进行报文转发
*输入参数: 无
*输出参数: 无
*返回参数: 无
***********************************************************/
int ChdDoSvr( )
{
    int  iRet ;
    int  iBufLen = 0;
    long lReqLen =0;
    char sSrcBuf[8096+1]={0};
    char sReqBuf[10240+1]={0};
    char sSvcName[TUX_NAME_LEN+1]={0};
    IPCMsgHead ipcMsgHead={0};
    sunda_req_head_t req_head={0};

    pid_t ppid = getppid();
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "comm_svr_%d.log", gSvrId);
    
    signal(SIGALRM,  TimeOut);
    signal(SIGTERM , svr_child_kill);
    
    SysLog(SYS_TRACE_NORMAL, "等待接受数据..."  );

    for (; ;)
    {
        memset(sSrcBuf, 0, sizeof(sSrcBuf));
        giTimeOut = 0;

        /*父进程死去,则退出循环*/
        if ( ppid != getppid() )
        {
        	SysLog(SYS_TRACE_ERROR, "parent death  ");
            close(giConnScktId);
            exit(1);
        }

        /* 取交易报文  */
        iRet = RcvTxnMsg(giConnScktId, SOCKET_MSG_HEAD_LEN, sSrcBuf, &iBufLen);
        if (iRet !=  0)
        {
        	if (errno ==  EINTR || iRet > 0)
        	{
        		continue;
        	}
        	
            SysLog(SYS_TRACE_ERROR, "RcvTxnMsg err [%d] ", iRet);
            close(giConnScktId);
            exit(1);
        }
        
        memset(&req_head, 0, sizeof(req_head));
        memcpy((char *)&req_head, sSrcBuf+SOCKET_MSG_HEAD_LEN, sizeof(req_head));
        memcpy(sSvcName, req_head.name, TUX_NAME_LEN);
        AllTrim(sSvcName);

       
        SysLog(SYS_TRACE_NORMAL , "read one msg ok ");
        
        memset(&ipcMsgHead, 0, sizeof(ipcMsgHead));
        strcpy(ipcMsgHead.name, sSvcName);
        ipcMsgHead.req_msg_type = COMM_L_MSG_TYPE_ROOT;
        ipcMsgHead.fd  = giConnScktId; 
        ipcMsgHead.ssn = req_head.ssn;
        ipcMsgHead.utime  = time(NULL);
        ipcMsgHead.reply_flag = REPLY_FLAG_ASYN;
        ipcMsgHead.len = iBufLen -  COMM_L_MSG_TYPE_ROOT - sizeof(req_head);
        memcpy(sReqBuf, (char *)&ipcMsgHead, sizeof(ipcMsgHead));
        memcpy(sReqBuf+sizeof(ipcMsgHead),  sSrcBuf+(SOCKET_MSG_HEAD_LEN + sizeof(req_head)), iBufLen);
        lReqLen = iBufLen+sizeof(ipcMsgHead);
        
        iRet = tux_acall(sSvcName, sReqBuf, lReqLen, 0);
        if (iRet != 0)
        {
        	SysLog(SYS_TRACE_ERROR , "tux_acall err %d !", tux_errno);
        	if (tux_errno == SYS_SYSTEM)
        	{
        		close(giConnScktId);
                exit(1);
        	}
        	continue;
        }
        
        SysLog(SYS_TRACE_ERROR , "tux_acall ok !");
    }

    return 0;
}

/**********************************************************
*函数名称: ChdDoClt
*函数功能: 子进程当客户端进行报文转发
*输入参数: 无
*输出参数: 无
*返回参数: 无
***********************************************************/
int ChdDoClt( )
{
    int   iRet;
    long  iBufLen = 0;
    int   iRspLen = 0;
    int   msg_type;
    char  sSrcBuf[10240+1]={0};
    char  sRspBuf[10240+1]={0};
    sunda_req_head_t req_head={0};
    IPCMsgHead ipcMsgHead={0};
    pid_t ppid = getppid();
    
    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "comm_clt_%d.log", gSvrId);

    signal(SIGTERM , clt_child_kill);
    
    for (; ;)
    {
        /*父进程死去,则退出循环*/
        if ( ppid != getppid() )
        {
        	SysLog(SYS_TRACE_ERROR, "parent death  ");
            close(giClntScktId);
            exit(1);
        }
               
        msg_type = COMM_L_MSG_TYPE_ROOT+gSvrId;
        memset(sSrcBuf, 0, sizeof(sSrcBuf));
        while (1)
        {
        	iRet = tux_getreply(msg_type, sSrcBuf, &iBufLen, 0);
            if (iRet != 0)
            {
            	if (errno == EINTR)
            	{
            		continue;
            	}
            	
            	SysLog(SYS_TRACE_ERROR , "sd_getreply iRet[%d]" , iRet);
            	close(giClntScktId);
                exit (1);
            }
            break;
        }

        memset(sRspBuf, 0, sizeof(sRspBuf));
        memset(&req_head, 0, sizeof(req_head));
        memcpy((char *)&ipcMsgHead, sSrcBuf, sizeof(ipcMsgHead));
        memcpy(req_head.name, ipcMsgHead.name, TUX_NAME_LEN);
        req_head.ssn = ipcMsgHead.ssn;
        req_head.err_no = 0;
        
        iRspLen = iBufLen-sizeof(gipcMsgHead);
        memcpy((char *)sRspBuf, (char *)&req_head, sizeof(req_head));
        memcpy(sRspBuf+sizeof(req_head), sSrcBuf+sizeof(gipcMsgHead), iRspLen);
        iRspLen = iRspLen + sizeof(req_head);

        /* 发送报文至目标系统 */
        alarm(1);
        iRet = WriteSocket(giClntScktId, sRspBuf, iRspLen);
        alarm(0);
        if (iRet < 0)
        {
            if (giTimeOut == 1)
            {
                /* 超时的情况 */
                continue;
            }
            else
            {
                SysLog(SYS_TRACE_ERROR , "WriteSocket iRet[%d]" , iRet );
                
                close(giClntScktId);
                exit(1);
            }
        }

        SysLog(SYS_TRACE_ERROR, " WriteSocket ok ");
    }

    return 0;
}

/**********************************************************
*函数名称: ChdMainProc
*函数功能: 子进程进行报文转发
*输入参数: iChdNum -- 进程编号
*输出参数: 无
*返回参数: 无
***********************************************************/
int ChdMainProc(int iChdNum)
{
    int iRet;

    /*  超时标志 */
    giTimeOut = 0;

    if (iChdNum %2 == 0)
    {
        iRet = ChdDoSvr();
    }
    else
    {
        iRet = ChdDoClt();
    }

    return iRet;
}

int main(int argc, char **argv)
{
    int    iRet, i;
    
    /* 进程初始化 */
    iRet = comm_init(argc, argv);
    if (iRet != 0)
    {
        SysLog(SYS_TRACE_ERROR,  "comm_init  err ");
        exit(1);
    }
    
    SysLog(SYS_TRACE_NORMAL , " -------- 等待客户端连接 ----------- ");

    while (1)
    {
    	/* 创建本地服务 */
        iRet = CreateSocket(giLocPort, 9999, 10, 0,&giLstnScktId);
        if (iRet != 0)
        {
            SysLog(SYS_TRACE_ERROR, "CreateSocket error iRet [%d]", iRet);
            exit(-1);
        }

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
            if (strcmp(gsCltIP, sRmtCltAddr) != 0)
            {
                close(giConnScktId);
                SysLog(SYS_TRACE_ERROR, "收到非法IP[%s]", sRmtCltAddr);
                continue;
            }

            break;
        }

        break;
    }

    for (i = 0; i < 11; i++)
    {
    	/* 连接远程服务端 */
    	iRet = OpenSocket(gsCltIP, giRmtPort, 9999, 10, &giClntScktId);
        if (iRet == 0)
        {
        	break;
        }

        if (i < 10)
        {
        	sleep(3);
            continue;
        }

        close(giConnScktId);
        close(giLstnScktId);
        SysLog(SYS_TRACE_ERROR, "OpenSocket error iRet [%d]", iRet);
        exit(-1);
    }

    SysLog(SYS_TRACE_NORMAL , "----------------链路状态连接正常-----------------");
    
    /*设置信号处理*/
    sigset(SIGCLD,  child_exit);
    sigset(SIGTERM, main_exit);
    
    sighold(SIGTERM);
    sighold(SIGCLD);
    
    /*------ fork进程池 ---- */
    for (i = 0 ; i < 2 ; i++)
    {
        if ((gpidtMainPid = fork()) == -1)
        {
            SysLog(SYS_TRACE_ERROR, "Call fork() err err [%d ][%s]", errno, strerror(errno));
            main_exit(1);
            exit(-1);
        }

        if (gpidtMainPid == 0)
        {
            close(giLstnScktId);
            ChdMainProc(i+1);
            exit(0);
        }
        
        gpidSonPid[i].pid  = gpidtMainPid;
        gpidSonPid[i].seq  = i+1;
    }
    
    sigrelse(SIGCLD);
    sigrelse(SIGTERM);

    while (1) 
    {
        pause();
    }

    return -1;
}

/******************************************* 文件结束 **********************************************/

