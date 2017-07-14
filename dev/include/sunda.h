#ifndef _SUNDA_H_
#define _SUNDA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "CfgOpr.h"
#include "IpcOpr.h"
#include "trace.h"
#include "task.h"
#include "errcode.h"
#include "tux.h"
#include "funmsq.h"
#include "funtime.h"
#include "funstr.h"
#include "funsock.h"

#define COMM_S_MSG_TYPE_ROOT    100000     /* 短链接通讯根消息类型*/
#define COMM_L_MSG_TYPE_ROOT    200000     /* 长链接通讯根消息类型*/
#define SVR_MSG_TYPE_ROOT       300000     /* 业务根消息类型*/
#define MNG_MSG_TYPE_ROOT       400000     /* 管理工具根消息类型*/
#define CMD_ROOT_MSG_TYPE       500000     /* 系统指令根消息类型 */


#define DEF_REQ_QUE             601107     /* 默认请求消息队列 */
#define DEF_RSP_QUE             601117     /* 默认应答消息队列 */


#define SD_MEM_NAME           "SD_MEN_NAME"
#define COMM_IPC_NAME         "COMM_SEM_NAME"
#define SD_SEM_NAME           "SD_SEM_NAME"
#define SD_SERVER_MGR_NAME    "Governor"
#define SD_MANAGER_SVC_NAME   "Governor"

#define SERVER_MAX_PARAM        128        /* task max param length */
#define MAX_TIMEOUT_SEC         20         /* default transaction timeout seconds */


#define MAGIC 0xfee1900d
#define IS_MAGIC(p) (p && *(long*)(((long*)p) - 1) == MAGIC)
#define MAX_MALLOC_LEN    5000
#define ALLOC_UNITS(s) (((s) + sizeof(struct tpmemory) - 1) / sizeof(struct tpmemory) + 2)

#define count_memory_size(nsvrnum, npidnum) \
          (sizeof(mb_ctrl_t)    + \
           sizeof(server_tab_t)    * (nsvrnum) + \
           sizeof(server_pid_t)     * (npidnum) + \
           sizeof(service_tab_t)    *  (nsvrnum))


typedef struct _mb_ctrl_t {
    int  nsvrnum;            /* max server number */
    int  npidnum;            /* max pid number */
    pid_t main_pid;          /* mbtask process id */
    int  semid;              /* sys-v sem */   
    int  memid;              /* sys-v shm */
    int  req_que;            /* 请求消息队列 */   
    int  rsp_que;            /* 应答消息队列 */
    int  req_que_id;         /* 请求消息队列句柄 */   
    int  rsp_que_id;         /* 应答消息队列句柄 */
    int  max_msg_len;        /* max msg buf len for mb */
} mb_ctrl_t;

typedef struct _server_tab_t {
    int           svrid;                          /* svr id */
    char          name[TUX_NAME_LEN + 1];         /* svr name */
    char          svc_name[TUX_NAME_LEN + 1];     /* svc name */
    char          parm[SERVER_MAX_PARAM + 1];     /* svr param */
    int           pid_tot;                        /* the svr process total count */
    int           state;                          /* 状态 */   
    int           rsp_q;                          /* 是否需要应答 */
    int           type;                           /* 进程退出类型 */ 
    long          msg_type;                       /* 读消息队列的消息类型 */ 
    int           timeout;                        /* 超时时间 */          
} server_tab_t;


typedef struct _server_pid_t {
    pid_t   pid;                           /* the server process id */
    int     svrid;                         /* the server tab index */
    int     svrpos;                        /* svr id in tab pos */
    int     seq;                           /* the task process sequence */ 
    int     restart_count_grace;           /* the restart count in restart grace */
    int	    tot_restart_count;             /* the total restart count */ 
    unsigned int  req_done;                /* 请求个数 */
    time_t  lastDie;                       /* the process last die time */
    time_t  lastStart;                     /* the process last start time */
} server_pid_t;


typedef struct {
    int  flags; 
    char name[TUX_NAME_LEN + 1];
    int  svrid;
    int (*sd_fun)(sd_info_t *);
} service_tab_t;

struct tpmemory {
    char *type, *subtype;
    long len, magic;
};

typedef struct {
    char name[TUX_NAME_LEN];
    unsigned int  ssn;
    int           err_no;
} sunda_req_head_t;


#define sdNumberOfSvr()        (mbctrl->nsvrnum)
#define sdNumberOfPid()        (mbctrl->npidnum)
#define sdMainPid()            (mbctrl->main_pid) 
#define sdReqQueId()           (mbctrl->req_que_id) 
#define sdRspQueId()           (mbctrl->rsp_que_id) 
#define sdIsAttach()           (mbctrl != NULL)
#define sdInvalidSvr(i)         ((i) < 0 || (i) >= mbctrl->svrnum)


#define sbSvrTabClear(i) do {memset(&tSvrTab[(i)], 0, sizeof(server_tab_t));tSvrTab[(i)].svrid = -1;} while (0)
#define sdGetSvrId(i)          (tSvrTab[(i)].svrid)  
#define sdGetSvrName(i)        (tSvrTab[(i)].name)
#define sdGetSvrSvcName(i)     (tSvrTab[(i)].svc_name)
#define sdGetSvrParam(i)       (tSvrTab[(i)].parm)
#define sdGetSvrNum(i)         (tSvrTab[(i)].pid_tot)
#define sdGetSvrStat(i)        (tSvrTab[(i)].state)
#define sdGetSvrRspFlag(i)     (tSvrTab[(i)].rsp_q)
#define sdGetSvrType(i)        (tSvrTab[(i)].type)
#define sdGetSvrMsgType(i)     (tSvrTab[(i)].msg_type)
#define sdGetSvrTimeOut(i)     (tSvrTab[(i)].timeout)

#define sdSvrPidClear(i) do {memset(&tSvrPid[(i)], 0, sizeof(server_pid_t)); tSvrPid[(i)].pid = -1; } while (0)
#define sdGetSvrPid(i)              (tSvrPid[(i)].pid)  
#define sdGetSvrPidId(i)            (tSvrPid[(i)].svrid) 
#define sdGetSvrPidPos(i)           (tSvrPid[(i)].svrpos) 
#define sdGetSvrPidSeq(i)           (tSvrPid[(i)].seq) 
#define sdGetSvrPidDone(i)          (tSvrPid[(i)].req_done)
#define sdGetSvrPidStarTime(i)      (tSvrPid[(i)].lastStart) 
#define sdGetSvrPidLastDie(i)       (tSvrPid[(i)].lastDie) 
#define sdGetSvrPidRestartNum(i)    (tSvrPid[(i)].tot_restart_count) 

#define sdGetSvcFlag(i)             (tSvcTab[(i)].flags) 
#define sdGetSvcName(i)             (tSvcTab[(i)].name)  
#define sdGetSvcId(i)               (tSvcTab[(i)].svrid) 
#define sdGetSvcFun(i)              (tSvcTab[(i)].sd_fun)   

#define SVR_PERM                1
#define SVR_TEMP                2
#define SVR_REPLY_NEED          1
#define SVR_REPLY_NONEED        2
#define SVC_INUSE               1  

        
#define TUX_AVAIL               1 
#define TUX_IDLE                2 
#define TUX_UNAVAIL             3 
#define TUX_DEAD                4  

extern mb_ctrl_t        *mbctrl;
extern server_tab_t     *tSvrTab;
extern server_pid_t     *tSvrPid;
extern service_tab_t    *tSvcTab;


#define TPNOBLOCK  MSQ_MODE_BLOCK
#define TPFAIL     TUX_FAIL
#define TPSUCCESS  TUX_SUCCESS
#define TPEXIT     TUX_EXIT
#define tpsvcinfo  _sd_info_t
#define TPSVCINFO  sd_info_t
#define TPEINVAL   SYS_INVAL
#define TPELIMIT   SYS_LIMIT
#define TPEOS      SYS_OS
#define TPEPROTO   SYS_PROTO
#define TPESVCFAIL SYS_SVCFAIL
#define TPESYSTEM  SYS_SYSTEM
#define TPETIME    SYS_TIMEOUT
#define TPGOTSIG   SYS_GOTSIG
#define TPENOENT   SYS_NOENT


#define tpalloc    tux_alloc
#define tprealloc  tux_realloc
#define tpfree     tux_free
#define tpadvertise tux_advertise
#define tpunadvertise tux_unadvertise
#define tpreturn   tux_return
#define tpcall     tux_call
#define tpacall    tux_acall
#define tpgetrply  tux_getreply
#define tpopen     DbsConnect
#define tpclose    DbsDisconnect
#define tpsvrinit  tux_svrinit
#define tpsvrdone  tux_svrdone
#define tperrno    tux_errno
#define tpstrerror tux_strerror
#define tplock     tux_lock
#define tpunlock   tux_unlock

#ifdef __cplusplus
}
#endif

#endif
