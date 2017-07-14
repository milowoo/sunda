#ifndef _FUNMSQ_H_
#define _FUNMSQ_H_
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "Type.h"


#define	MSQ_MODE_PERM   ((mode_t) 0777)

#define  MSQ_CHK_YES    1
#define  MSQ_CHK_NO     0

#define  BUS_MAX_MSG_LEN    4060 
#define  CLNT_SVC_LEN       32
#define  MSQ_MAX_NUM        1024


#define MSQ_MODE_BLOCK		1
#define MSQ_MODE_NONBLOCK	2

#define REPLY_FLAG_SYN          1
#define REPLY_FLAG_ASYN         2


typedef struct
{
	char            name[CLNT_SVC_LEN + 1];            /* ������õ�service name */
	unsigned int    ssn;                               /* �ͻ���������ˮ */
	unsigned int    conn_ssn;                          /* ͨѶ��ˮ */
    long            req_msg_type;                      /* ����Ľ��̱�� */
    int             fd;                                /* connect client fd */
    UINT64          utime;                             /* onnect time */
    int             reply_flag;                        /* Ӧ���־ 1- ͬ�� 2 - �첽 */
    long            note_msg_type;                     /* �첽����ʱʹ�� */
    long            len;                               /* request data length */
}IPCMsgHead;


extern IPCMsgHead gipcMsgHead;


typedef struct{
    int    nChkFlag;    /*�Ƿ�����Ϣ����*/
	int    nMsqMaxNum;  /*��Ϣ���������,�����ǰ��Ϣ������������Ͳ�����*/
	int    nWaitFlag;   /*���͵ȴ���־, ��Ϣ��������ʱ���Ƿ�ȴ�*/
}IPCChkMsq;

typedef struct{
	long    nMsgType;
	long 	nMsgSource;
	char    sBuf[BUS_MAX_MSG_LEN];
}IPCQueMsg;



/*********************************************************************
*��������: MsqCreate
*��������: ������Ϣ����
*�������: key  -- ���
*�������: ��
*�� �� ֵ: > 0      -- �ɹ�
           <= 0     -- ʧ��
***********************************************************************/
extern int MsqCreate (key_t key);


/*********************************************************************
*��������: MsqGet
*��������: ȡ����Ϣ���о��
*�������: key  -- ���
*�������: ��
*�� �� ֵ: > 0      -- �ɹ�
           <= 0     -- ʧ��
***********************************************************************/
extern int MsqGet (key_t key);


/*********************************************************************
*��������: MsqRcv
*��������: ȡ��Ϣ������Ϣ
*�������: nMsqId    -- ��Ϣ����ID
           nMsgLen   -- ��Ϣ������󳤶�
           nMsgType  -- ��Ϣ��������
           iRcvMod   -- ��ȡ����  MSQ_MODE_BLOCK/ MSQ_MODE_NONBLOCK
*�������: ��
*�� �� ֵ: > 0  -- �ɹ� ,������Ϣ����
           <=0  -- ʧ��
***********************************************************************/
extern int MsqRcv(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, long nMsgType, int iRcvMod);


/*********************************************************************
*��������: MsqSnd
*��������: ������Ϣ����
*�������: nMsqId      -- ��Ϣ����ID
           nMsgLen     -- ��Ϣ������󳤶�
           pipcChkMsq  -- �����Ϣ���нṹ
*�������: ��
*�� �� ֵ: 0    -- �ɹ�
           ���� -- ʧ��
***********************************************************************/
extern int MsqSnd(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, IPCChkMsq *pipcChkMsq);



/*********************************************************************
*��������: MsqCount
*��������: ͳ����Ϣ������Ϣ����
*�������: nMsqId   -- ��Ϣ����ID
*�������: ��
*�� �� ֵ: >=0    -- ��Ϣ����
           ����   -- ʧ��
***********************************************************************/
extern int MsqCount(int nMsqId);


/*********************************************************************
*��������: MsqClear
*��������: �����Ϣ����
*�������: nMsqId   -- ��Ϣ����ID
*�������: ��
*�� �� ֵ: 0      -- �ɹ�
           ����   -- ʧ��
***********************************************************************/
extern int MsqClear(int nMsqId);


/*********************************************************************
*��������: MsqClose
*��������: �ر���Ϣ����
*�������: nMsqId   -- ��Ϣ����ID
*�������: ��
*�� �� ֵ: 0      -- �ɹ�
           ����   -- ʧ��
***********************************************************************/
extern int MsqClose(int nMsqId);


#endif
