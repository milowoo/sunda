/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：消息队列封装函数
 *
 *  Edit History:
 *
 *    2012/03/12 - MiloWoo 建立.
 *************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <sys/msg.h>
#include "funmsq.h"
//#include "trace.h"


int MsqCreate (key_t key)
{
	return (msgget (key, IPC_CREAT | MSQ_MODE_PERM));

} /* end of MsqCreate() */


int MsqGet (key_t key)
{
	return (msgget (key, MSQ_MODE_PERM));
}

int MsqRcv(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, long nMsgType, int iRcvMod)
{
	long       lMsgFlag = 0;
    int        code;
    
    if (iRcvMod == MSQ_MODE_NONBLOCK)
    {
    	lMsgFlag = IPC_NOWAIT;
    }

    code = msgrcv(nMsqId, ipcMsgIn, nMsgLen , nMsgType, lMsgFlag);
    return code - sizeof(long);
}

int MsqSnd(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, IPCChkMsq *pipcChkMsq)
{
    int                code, msgflg;
    struct msqid_ds    msqInf;

    if (pipcChkMsq->nChkFlag == MSQ_CHK_YES)
    {
        /* check queue state */
        code = msgctl(nMsqId, IPC_STAT, &msqInf);
        if (code == -1)
            return errno;

        /* check number of msg in the queue */
        if (pipcChkMsq->nMsqMaxNum == 0) pipcChkMsq->nMsqMaxNum = MSQ_MAX_NUM;
        if (msqInf.msg_qnum > pipcChkMsq->nMsqMaxNum)
            return -1;
    }
    
    if (nMsgLen >= BUS_MAX_MSG_LEN)
    {
    	return -2;
    }

    /* send msg into queue */
    msgflg =  (pipcChkMsq->nWaitFlag == MSQ_MODE_NONBLOCK) ? IPC_NOWAIT : 0;
    	
    code = msgsnd(nMsqId, ipcMsgIn, nMsgLen + sizeof(long), msgflg);
    if (code == -1)
        return errno;

    return 0;
}

int MsqCount (int msqid)
{

	struct msqid_ds qds;

	if (msgctl (msqid, IPC_STAT, &qds) == -1) return (-1); /* failure */

	return (qds.msg_qnum); /* number of messages on queue */

}

int MsqClear (int msqid)
{

	IPCQueMsg msgbuf;
	int flag; /* return flag of MsqCount() */

	/* read out all messages on queue */
	flag = MsqCount (msqid);
	if (flag < 0) return (-1); /* failure */
	while (flag > 0)
	{
		if (MsqRcv (msqid, &msgbuf, BUS_MAX_MSG_LEN , 0, MSQ_MODE_NONBLOCK) < 0)
			return (-1); /* failure */
		else
			printf("msg_type =[%ld]  \n", msgbuf.nMsgType);
			
		flag --;
	}
	return (0); /* success */

} /* end of MsqClear() */




int MsqClose (int msqid)
{
	struct msqid_ds qds;

	return (msgctl (msqid, IPC_RMID, &qds));

}


/**************************************  文件结束 *****************************/
