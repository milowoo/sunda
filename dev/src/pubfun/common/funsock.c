/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  ���ܣ�socket ������װ����
 *
 *  Edit History:
 *
 *    2012/03/12 - MiloWoo ����.
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
*��������: IOTimeOut
*��������: ���ܳ�ʱ�źź���
*�������: no -- ������
*�������  ��
*��������: ��
*****************************************************/
static void IOTimeOut(int no)
{
    Timeout = 1;
    siglongjmp(Stackbuf, Timeout);
    return;
}

/*****************************************************
*��������: sock_set_block
*��������: ��socket �趨������ģʽ
*�������: sockid     -- socket id
���������� on  -- �趨ģʽ 0 �Ƕ��� 1 ����
*�������  ��
*��������: 0  -- �ɹ�
           -1 -- ʧ��
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
*��������: CreateSocket
*��������: ����socket,Listen�˿�
*�������: nPort     -- �˿ں�
���������� nRetryNum -- ʧ��ʱ�ظ��Ĵ���
           nWaitTime -- ʧ��ʱ�ظ��ȴ�ʱ��
           nBlock    -- ������־ 1 -- �Ƕ��� 0 - ����
*�������  nSocket   -- socket������
*��������: 0  -- �ɹ�
           -1 -- ʧ��
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
*��������: ReadSocket
*��������: ��socketָ�����ֽ�
*�������: nSocket  -- socket������
���������� nLen���� -- �ƶ���socket����
*�������  sBuf��   -- ��socket����
*��������: 0  -- �ɹ�
           -1 -- ʧ��
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
*��������: WriteSocket
*��������: дsocketָ�����ֽ�
*�������: nSocket  -- socket������
���������� sBuf     -- дsocket����
           nLen     -- дsocket����
*�������  ��
*��������: 0  -- �ɹ�
           -1 -- ʧ��
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
*��������: Setsokopt
*��������: ����socket,ʹsocket���Ӹ���׳
*�������: nSocket  -- socket������
*�������  ��
*��������: 0  -- �ɹ�
           -1 -- ʧ��
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
*��������: AcceptSocket
*��������: ����Զ�˿ͻ��˵�����
*�������: nSocket    -- socket������
           nWaitTime  -- ϵͳ����ȴ�ʱ��
           nNewSocket -- ��socket������
*�������  client���� -- �ͻ�����Ϣ
*��������: 0  -- �ɹ�
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
*��������: OpenSocket
*��������: ����Զ��socket����
*�������: sIpAddr�� -- Զ��socket���� IP ��ַ
���������� nPort     -- ���������˿ں�
           nRetrys   -- ������������
           nTimeOut  -- ���ó�ʱʱ��
*�������  nSocket�� -- �����׽���
*��������: 0  -- �ɹ�
����������<0  -- ʧ��
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
*��������: CloseSocket
*��������: �ر�socket��
*�������: nSocket  -- Ҫ�رյ�Socket��
*�������  ��
*��������: 0  -- �ɹ�
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
*��������: RcvTxnMsg
*��������: ͨ��SOCKET��ȡ���ױ���
*�������: iSocketId    -- ��Դϵͳ��socket ������
           iHeadLen     -- ����ͷ����
*�������: sSrcBuf      -- ���ױ���
           iBufLen      -- ���ױ��ĳ���
*���ز���: 0    -- �ɹ�
           1    -- ���Կձ���
           ���� -- ʧ��
***********************************************************/
int  RcvTxnMsg(int iSocketId, int iHeadLen, char *sSrcBuf, int * iBufLen)
{
    int  iRet;
    int  iMsgLen=0;
    int  iMsgInLen=0;
    int  iTcpMsgLen = 0;
    char sMsgLen[128 + 1]={0};
    char sMsgInBuf[10240+1]={0};

    /* �ȶ�����ͷ */
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

/**************************************************** �ļ����� ********************************/
