/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：socket 操作封装函数
 *
 *  Edit History:
 *
 *    2012/03/12 - MiloWoo 建立.
 *************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <setjmp.h>
#include <sys/types.h>
#include <signal.h>
#include "trace.h"

static Timeout = 0;
jmp_buf Stackbuf;

static void IOTimeOut(int no);


/*****************************************************
*函数名称: IOTimeOut
*函数功能: 接受超时信号函数
*输入参数: no -- 无意义
*输出参数  无
*函数返回: 无
*****************************************************/
static void IOTimeOut(int no)
{
    Timeout = 1;
    siglongjmp(Stackbuf, Timeout);
    return;
}

/*****************************************************
*函数名称: sock_set_block
*函数功能: 给socket 设定非阻塞模式
*输入参数: sockid     -- socket id
　　　　　 on  -- 设定模式 0 非堵塞 1 堵塞
*输出参数  无
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
int sock_set_block(int sockid, int on)
{
    int val;
    int rv;

    val = fcntl(sockid, F_GETFL, 0);
    if (on) 
    {
        rv = fcntl(sockid, F_SETFL, ~O_NONBLOCK&val);
    } 
    else {
        rv = fcntl(sockid, F_SETFL, O_NONBLOCK|val);
    }
    if (rv) {
        return errno;
    }

    return 0;
}

int  setnonblocking(int sockid)
{
    int opts;
    
    fcntl(sockid, F_SETFL, O_NONBLOCK); 
    
#if 0    
    opts=fcntl(sockid, F_GETFL);
    if(opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        return -3;
    }
    
    opts = opts|O_NONBLOCK;
    
    if(fcntl(sockid, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return -4;
    }
#endif
}



/*****************************************************
*函数名称: CreateSocket
*函数功能: 建立socket,Listen端口
*输入参数: nPort     -- 端口号
　　　　　 nRetryNum -- 失败时重复的次数
           nWaitTime -- 失败时重复等待时间
           nBlock    -- 堵塞标志 1 -- 非堵塞 0 - 堵塞
*输出参数  nSocket   -- socket描述符
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
int CreateSocket(unsigned int nPort, int nRetryNum, int nWaitTime, int nBlock, int *nSocket)
{
	int iRet; 
    int     nNum = 0;
    struct  sockaddr_in LocalAddr;

    nNum = nRetryNum;
    while ((*nSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        if (nRetryNum-- <= 0)
        {
            return -1;
        }

        sleep(nWaitTime);
    }

    Setsokopt(*nSocket);
    
    memset(&LocalAddr, 0, sizeof(LocalAddr));
    LocalAddr.sin_port = htons(nPort);
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    nRetryNum = nNum;
    while (bind(*nSocket,(struct sockaddr *)&LocalAddr, sizeof(LocalAddr)) <0)
    {
        if (nRetryNum-- <= 0)
        {
            CloseSocket(*nSocket);
            return -2;
        }

        sleep(nWaitTime);
    }
    
    if (nBlock == 1)
    {
    	iRet = setnonblocking(*nSocket);
    	if (iRet != 0)
    	{
    		CloseSocket(*nSocket);
    		return -3;
    	}
    	// sock_set_block(*nSocket, 0);
    }

    listen(*nSocket, 400);

    return 0;
}

/*****************************************************
*函数名称: ReadSocket
*函数功能: 读socket指定个字节
*输入参数: nSocket  -- socket描述符
　　　　　 nLen　　 -- 制定读socket长度
*输出参数  sBuf　   -- 读socket内容
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
int ReadSocket(int nSocket, char *sBuf, int nLen)
{
    int     nReadLen = 0, nNum = 0;

    if (nLen <= 0)
    {
        return -1;
    }

    /* alarm(5); */
    do
    {
        nNum = read(nSocket, sBuf + nReadLen, nLen - nReadLen);
        if (nNum <= 0)
        {
            return -1;
        }

        nReadLen += nNum;
    }
    while (nReadLen < nLen);

    /* alarm(0); */

    if (nReadLen < nLen)
    {
        return -1;
    }

    return 0;
}

/*****************************************************
*函数名称: WriteSocket
*函数功能: 写socket指定个字节
*输入参数: nSocket  -- socket描述符
　　　　　 sBuf     -- 写socket内容
           nLen     -- 写socket长度
*输出参数  无
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
int WriteSocket(int nSocket, char *sBuf, int nLen)
{
    int     nNum = 0;
    int     nWriteLen = 0;

    if (nLen <= 0)
    {
        return -1;
    }

    for (;;)
    {
        while ((nNum = write(nSocket, sBuf + nWriteLen, nLen - nWriteLen)) <= 0)
        {
            /* if (errno == EINTR) continue; */
            return -1;
        }

        nWriteLen += nNum;
        if (nWriteLen >= nLen) break;;
    }

    return 0;
}

/*****************************************************
*函数名称: Setsokopt
*函数功能: 设置socket,使socket连接更健壮
*输入参数: nSocket  -- socket描述符
*输出参数  无
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
static int Setsokopt(int nSocket)
{
    struct linger Linger;
    int    nKeepLive, nReuseAddr;

    Linger.l_onoff = 1;
    Linger.l_linger = 0;
    if (-1 == setsockopt(nSocket, SOL_SOCKET, SO_LINGER, (char*)&Linger, (int)sizeof(Linger)))
    {
        return -1;
    }

   // nKeepLive = 0;
    nKeepLive = 1;
    if (-1 == setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, &nKeepLive, (int)sizeof(nKeepLive)))
    {
        return -1;
    }

    nReuseAddr = 1;
    if (-1 == setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, (int)sizeof(nReuseAddr)))
    {
        return -1;
    }

    return 0;
}


/*****************************************************
*函数名称: AcceptSocket
*函数功能: 接收远端客户端的连接
*输入参数: nSocket    -- socket描述符
           nWaitTime  -- 系统出错等待时间
           nNewSocket -- 新socket描述符
*输出参数  client　　 -- 客户端信息
*函数返回: 0  -- 成功
*****************************************************/
int AcceptSocket(int nSocket, struct sockaddr_in *client, int nWaitTime, int *nNewSocket)
{
	unsigned int nAddrLen = sizeof(struct sockaddr_in);

    while (1)
    {
        *nNewSocket = accept(nSocket, client, &nAddrLen);
		if (*nNewSocket >= 0)
		{
		    break;
		}

        if (errno == EINTR)
        {
            continue;
        }
        else
        {
            sleep(nWaitTime);
            continue;
        }
    }

    return 0;
}


/*****************************************************
*函数名称: OpenSocket
*函数功能: 连接远程socket服务
*输入参数: sIpAddr　 -- 远程socket主机 IP 地址
　　　　　 nPort     -- 连接主机端口号
           nRetrys   -- 允许重连次数
           nTimeOut  -- 设置超时时间
*输出参数  nSocket　 -- 连接套接字
*函数返回: 0  -- 成功
　　　　　<0  -- 失败
*****************************************************/
int OpenSocket(char *sIpAddr, int nPort, int nRetrys, int nTimeOut, int *nSocket)
{
    struct sockaddr_in remote;

    bzero(&remote, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(sIpAddr);
    remote.sin_port = htons(nPort);

    Timeout = 0;
    signal(SIGALRM, IOTimeOut);
    alarm(nTimeOut);

    sigsetjmp(Stackbuf, 1);
    if (Timeout)
    {
        alarm(0);
        CloseSocket(*nSocket);
        signal(SIGALRM, SIG_DFL);
        return -1;
    }

    while (nRetrys)
    {
        if ((*nSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            if (nRetrys > 0)
            {
                nRetrys--;
                continue;
            }

            alarm(0);
            signal(SIGALRM, SIG_DFL);
            return -2;
        }

        if (connect(*nSocket, (struct sockaddr *)&remote, sizeof(remote)) < 0)
        {
            CloseSocket(*nSocket);
            if (nRetrys > 0)
            {
                nRetrys--;
                continue;
            }

            alarm(0);
            signal(SIGALRM, SIG_DFL);
            return -3;
        }

        alarm(0);
        Setsokopt(*nSocket);
        signal(SIGALRM, SIG_DFL);
        return 0;

    } /* while */

   	alarm(0);
    signal(SIGALRM, SIG_DFL);
   	return -1;
}

/*****************************************************
*函数名称: CloseSocket
*函数功能: 关闭socket号
*输入参数: nSocket  -- 要关闭的Socket号
*输出参数  无
*函数返回: 0  -- 成功
*****************************************************/
int CloseSocket(int nSocket)
{
   /*  if (fcntl(nSocket, F_SETFL, FNDELAY) < 0) */
    if (fcntl(nSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        return 1;
    }

    if (nSocket != 0)
    {
        shutdown(nSocket, 2);
    }

    close(nSocket);

    return 0;
}


/**********************************************************
*函数名称: RcvTxnMsg
*函数功能: 通过SOCKET读取交易报文
*输入参数: iSocketId    -- 与源系统的socket 连接字
           iHeadLen     -- 报文头长度
*输出参数: sSrcBuf      -- 交易报文
           iBufLen      -- 交易报文长度
*返回参数: 0    -- 成功
           1    -- 测试空报文
           其他 -- 失败
***********************************************************/
int  RcvTxnMsg(int iSocketId, int iHeadLen, char *sSrcBuf, int * iBufLen)
{
    int  iRet;
    int  iMsgLen=0;
    int  iMsgInLen=0;
    int  iTcpMsgLen = 0;
    char sMsgLen[128 + 1]={0};
    char sMsgInBuf[10240+1]={0};

    /* 先读报文头 */
    iRet = ReadSocket(iSocketId, sMsgLen, iHeadLen);
    if (iRet != 0)
    {
        return -1;
    }
    
    iMsgInLen = atoi(sMsgLen);
    
    if (iMsgInLen <= 0)
    {
    	return 1;
    }

    memset(sMsgInBuf, 0, sizeof(sMsgInBuf));
    iRet = ReadSocket(iSocketId, sMsgInBuf, iMsgInLen);
    if (iRet != 0)
    {
        return -1;
    }


    memcpy(sSrcBuf, sMsgLen, iHeadLen);
    memcpy(sSrcBuf+iHeadLen, sMsgInBuf, iMsgInLen);

    *iBufLen = iHeadLen + iMsgInLen;

    return 0;
}

/**************************************************** 文件结束 ********************************/
