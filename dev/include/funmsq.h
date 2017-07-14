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
	char            name[CLNT_SVC_LEN + 1];            /* 请求调用的service name */
	unsigned int    ssn;                               /* 客户端请求流水 */
	unsigned int    conn_ssn;                          /* 通讯流水 */
    long            req_msg_type;                      /* 处理的进程编号 */
    int             fd;                                /* connect client fd */
    UINT64          utime;                             /* onnect time */
    int             reply_flag;                        /* 应答标志 1- 同步 2 - 异步 */
    long            note_msg_type;                     /* 异步调用时使用 */
    long            len;                               /* request data length */
}IPCMsgHead;


extern IPCMsgHead gipcMsgHead;


typedef struct{
    int    nChkFlag;    /*是否检查消息队列*/
	int    nMsqMaxNum;  /*消息队列最大数,如果当前消息数超过最大数就不发送*/
	int    nWaitFlag;   /*发送等待标志, 消息队列满的时候是否等待*/
}IPCChkMsq;

typedef struct{
	long    nMsgType;
	long 	nMsgSource;
	char    sBuf[BUS_MAX_MSG_LEN];
}IPCQueMsg;



/*********************************************************************
*函数名称: MsqCreate
*函数功能: 创建消息队列
*输入参数: key  -- 句柄
*输出参数: 无
*返 回 值: > 0      -- 成功
           <= 0     -- 失败
***********************************************************************/
extern int MsqCreate (key_t key);


/*********************************************************************
*函数名称: MsqGet
*函数功能: 取得消息队列句柄
*输入参数: key  -- 句柄
*输出参数: 无
*返 回 值: > 0      -- 成功
           <= 0     -- 失败
***********************************************************************/
extern int MsqGet (key_t key);


/*********************************************************************
*函数名称: MsqRcv
*函数功能: 取消息队列信息
*输入参数: nMsqId    -- 消息队列ID
           nMsgLen   -- 消息队列最大长度
           nMsgType  -- 消息队列类型
           iRcvMod   -- 读取类型  MSQ_MODE_BLOCK/ MSQ_MODE_NONBLOCK
*输出参数: 无
*返 回 值: > 0  -- 成功 ,返回消息长度
           <=0  -- 失败
***********************************************************************/
extern int MsqRcv(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, long nMsgType, int iRcvMod);


/*********************************************************************
*函数名称: MsqSnd
*函数功能: 发送消息队列
*输入参数: nMsqId      -- 消息队列ID
           nMsgLen     -- 消息队列最大长度
           pipcChkMsq  -- 检查消息队列结构
*输出参数: 无
*返 回 值: 0    -- 成功
           其他 -- 失败
***********************************************************************/
extern int MsqSnd(int nMsqId, IPCQueMsg *ipcMsgIn, int nMsgLen, IPCChkMsq *pipcChkMsq);



/*********************************************************************
*函数名称: MsqCount
*函数功能: 统计消息队列消息个数
*输入参数: nMsqId   -- 消息队列ID
*输出参数: 无
*返 回 值: >=0    -- 消息个数
           其他   -- 失败
***********************************************************************/
extern int MsqCount(int nMsqId);


/*********************************************************************
*函数名称: MsqClear
*函数功能: 清空消息队列
*输入参数: nMsqId   -- 消息队列ID
*输出参数: 无
*返 回 值: 0      -- 成功
           其他   -- 失败
***********************************************************************/
extern int MsqClear(int nMsqId);


/*********************************************************************
*函数名称: MsqClose
*函数功能: 关闭消息队列
*输入参数: nMsqId   -- 消息队列ID
*输出参数: 无
*返 回 值: 0      -- 成功
           其他   -- 失败
***********************************************************************/
extern int MsqClose(int nMsqId);


#endif
