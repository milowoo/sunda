/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能: 框架调用函数
 *
 *  Edit History:
 *
 *    2012/03/12 - MiloWoo 建立.
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include "sunda.h"

int tux_errno = 0;

static int _mb_longjmp;
static jmp_buf _mb_jmpbuf;

mb_ctrl_t     *mbctrl;
server_tab_t  *tSvrTab;    /* array for server  */  
server_pid_t  *tSvrPid;    /* array for pid  */  
service_tab_t *tSvcTab;    /* array for service functions */

IPCMsgHead gipcMsgHead;

static int timer_expired;       /* timeout flag */

static char *emptystring = "";
static char *stringtype = "STRING";
static char *carraytype = "CARRAY";

void printfIPC()
{
	SysLog(SYS_TRACE_ERROR,"semid [%d] memid [%d] req_que [%d] rsp_que [%d] req_que_id [%d] rsp_que_id [%d]", 
	       mbctrl->semid, mbctrl->memid,  mbctrl->req_que, mbctrl->rsp_que, mbctrl->req_que_id, mbctrl->rsp_que_id);
	
	return ;
}


char *tux_alloc(const char *type, const char *subtype, long size)
{
    struct tpmemory *p, *p2;
    long len ;

    if (size < 0) 
    {
        tux_errno = SYS_INVAL;
        return NULL;
    }

    len = size + 2048;
    
    p = malloc(ALLOC_UNITS(len) * sizeof(struct tpmemory));
    if (p == 0) 
    {
        tux_errno = SYS_OS;
        return NULL;
    }

    p2 = p + ALLOC_UNITS(len) - 1;
    if (strcmp(type, stringtype) == 0)
        p2->type = p->type = stringtype;
    else if (strcmp(type, carraytype) == 0)
        p2->type = p->type = carraytype;
    else
        p2->type  = p->type = emptystring;
    p2->subtype = p->subtype = emptystring;
    p2->len   = p->len   = len;
    p2->magic = p->magic = MAGIC;
    return (char *)(p + 1);
}

char *tux_realloc(char *ptr, long size)
{
    struct tpmemory *p, *p2;

    if (!IS_MAGIC(ptr) || size < 0) {
        tux_errno = SYS_INVAL;
        return NULL;
    }

    p  = (struct tpmemory *)ptr - 1;
    p2 = p + ALLOC_UNITS(p->len) - 1;

    if (p->len != p2->len || p->magic != p2->magic ||
        p->type != p2->type || p->subtype != p2->subtype) {
        tux_errno = SYS_INVAL;
        return NULL;
    }

    p = realloc(p, ALLOC_UNITS(size) * sizeof(struct tpmemory));
    if (p == 0) {
        tux_errno = SYS_OS;
        return NULL;
    }

    p2 = p + ALLOC_UNITS(size) - 1;
    p2->len = p->len = size;
    p2->type = p->type;
    p2->subtype = p->subtype;
    p2->magic = p->magic;

    return (char *)(p + 1);
}

void tux_free(char *ptr)
{
    struct tpmemory *p, *p2;

    if (ptr == NULL)
        return;

    if (!IS_MAGIC(ptr))
        return;

    p  = (struct tpmemory *)ptr - 1;
    p2 = p + ALLOC_UNITS(p->len) - 1;

    if (p->len != p2->len || p->magic != p2->magic ||
        p->type != p2->type || p->subtype != p2->subtype)
        return;

    p2->magic = p->magic = ~MAGIC;
    free(p);
}

static void tux_timefunc(int n)
{
    timer_expired = 1;
    SysLog(SYS_TRACE_NORMAL,"read msg timeout");
}

static void tux_settimer(int timer)
{
    timer_expired = 0;
    sigset(SIGALRM, tux_timefunc);
    alarm(timer);
}

static void tux_unsettimer()
{
    alarm(0);
}

static int tux_timer_expired()
{
    return timer_expired;
}

int tux_lock()
{
    return sem_lock(0, mbctrl->semid);
}

int tux_unlock()
{
    return sem_unlock(0, mbctrl->semid);
}

static int que_remove()
{
    MsqClose(mbctrl->req_que_id);
    MsqClose(mbctrl->rsp_que_id);
    return 0;
}

static int stop_system()
{
    long pid;

    if (sdIsAttach()) 
    {
        pid = sdMainPid();
        if (pid <= 0) 
        {
            fprintf(stderr, "task manager process not stop properly..\n");
        } 
        else 
        {
            printf("Stopping task manager pid: %ld\n", pid);
            kill(pid, SIGTERM);
        }
    }

    return 0;
}

int ipc_remove()
{
    stop_system();
    sleep(5);
    
    printf("Removing Resources:\n");
    
    que_remove();
    
    sem_delete(SD_SEM_NAME, 1);
    
    shm_delete(SD_MEM_NAME, (const char*)mbctrl);
    
    printf("\nMessage Queue system down.\n");

    return 0;
}

int createqueue(int req_que, int rsp_que)
{
	mbctrl->req_que = req_que;
	mbctrl->rsp_que = rsp_que;
	
    mbctrl->req_que_id = MsqCreate (mbctrl->req_que);
    if (mbctrl->req_que_id < 0)
    {
    	SysLog(SYS_TRACE_ERROR,"create req que err error %d %s",  errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    mbctrl->rsp_que_id =  MsqCreate (mbctrl->rsp_que);
    if (mbctrl->rsp_que_id < 0)
    {
    	SysLog(SYS_TRACE_ERROR,"create rsp_que err error %d %s", errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }

    return 0;
}

int que_connect()
{
	mbctrl->req_que_id = MsqGet (mbctrl->req_que);
    if (mbctrl->req_que_id < 0)
    {
    	SysLog(SYS_TRACE_ERROR,"MsqGet req que err error %d %s",  errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    mbctrl->rsp_que_id = MsqGet (mbctrl->rsp_que);
    if (mbctrl->rsp_que < 0)
    {
    	SysLog(SYS_TRACE_ERROR,"MsqGet rsp_que err error %d %s", errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
	
}

int tux_createsem()
{
    mbctrl->semid = sem_create(SD_SEM_NAME, 1);
    if (mbctrl->semid < 0) {
        SysLog(SYS_TRACE_ERROR,"sem_create %s error %d %s", SD_SEM_NAME, errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    return 0;
}

int sys_start()
{
    int i, rc;
    int npidnum, nsvrnum, nreq_que, nrsp_que, nmsglen;
    unsigned long size;
    FILE *fpcfg;

    fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG, errno, strerror(errno));
        tux_errno = SYS_SYSTEM;
        return -1;
    }
    
    /* 最多服务个数 */
    if (cf_locatenum(fpcfg, "sd.svrnum", &nsvrnum) < 0 || nsvrnum <= 0)
        nsvrnum = 80;  
    /* 最多进程 个数 */
    if (cf_locatenum(fpcfg, "sd.pidnum", &npidnum) < 0 || npidnum <= 0)
        npidnum = 800; 
    /* 消息长度限制 */
    if (cf_locatenum(fpcfg, "sd.maxmsglen", &nmsglen) < 0 || nmsglen <= 0)
        nmsglen = 4096; 
    /* 系统请求消息队列 */
    if (cf_locatenum(fpcfg, "sd.req_que", &nreq_que) < 0 || nreq_que <= 0)
        nreq_que = DEF_REQ_QUE;
    /* 系统应答消息队列 */
    if (cf_locatenum(fpcfg, "sd.rsp_que", &nrsp_que) < 0 || nrsp_que <= 0)
        nrsp_que = DEF_RSP_QUE;
    
    cf_close(fpcfg);

    /* 判断共享内存是否已经存在 */
    mbctrl = (mb_ctrl_t*)shm_connect(SD_MEM_NAME);
    if (mbctrl != NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"shm %s already exist, please shutdown it", SD_MEM_NAME);
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    /* create total shared memory */
    size = count_memory_size(nsvrnum, npidnum) ;
    SysLog(SYS_TRACE_ERROR,"shm_create size %d ", size);
    mbctrl = (mb_ctrl_t *)shm_create(SD_MEM_NAME, size);
    if (mbctrl == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"shm_create %s %uld error %d %s", SD_MEM_NAME, size, errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    memset((void*)mbctrl, 0, size);
    mbctrl->nsvrnum     = nsvrnum;
    mbctrl->npidnum     = npidnum;
    mbctrl->memid       = shm_getid(SD_MEM_NAME, 0, 0);
    mbctrl->max_msg_len = nmsglen;
    mbctrl->main_pid       = getpid();

    tSvrTab   = (server_tab_t *)&mbctrl[1];
    tSvrPid   = (server_pid_t *)&tSvrTab[mbctrl->nsvrnum];
    tSvcTab   = (service_tab_t *)&tSvrPid[mbctrl->npidnum]; 
    
    /* 初始化共享内存 */
    for (i = 0; i < sdNumberOfSvr(); i++) 
    {
        sdGetSvrId(i) = sdGetSvcFlag(i) = -1;
        memset(sdGetSvcName(i), 0, TUX_NAME_LEN + 1);
    }
    
    for (i = 0; i < sdNumberOfPid(); i++)
    {
    	sdGetSvrPid(i)  = -1;
    }
    
    /* queue  for mb task */
    rc = createqueue(nreq_que, nrsp_que);
    if (rc) 
    {
    	shm_delete(SD_MEM_NAME, (const char*)mbctrl);
        SysLog(SYS_TRACE_ERROR,"createqueue error %d %s", errno, strerror(errno));
        return rc;
    }
    
    mbctrl->semid = sem_create(SD_SEM_NAME, 1);
    if (mbctrl->semid < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"sem_create %s error %d %s", SD_SEM_NAME, errno, strerror(errno));
        tux_errno = SYS_OS;
        shm_delete(SD_MEM_NAME, (const char*)mbctrl);
        que_remove();
        return -1;
    }
    
    /* register server manager info */
    strcpy(sdGetSvrName(0), SD_SERVER_MGR_NAME);
    strcpy(sdGetSvrSvcName(0), SD_MANAGER_SVC_NAME); 
    sdGetSvrNum(0) = 1;
    sdGetSvrId(0) = 0;
    sdGetSvrStat(0) = TUX_IDLE;
    
    sdGetSvrPid(0) = getpid();
    sdGetSvrPidId(0) = 0;
    sdGetSvrPidSeq(0) = 0;
    sdGetSvrPidPos(0) = 0;
    sdGetSvrPidStarTime(0) = time(NULL);
    sdGetSvrPidDone(0) = 0;
    sdGetSvrMsgType(0) = SVR_MSG_TYPE_ROOT+sdGetSvrId(0);
    sdGetSvrTimeOut(0) = -1; /* no limit */
    
    return 0;
}

int ipc_connect()
{
    int rc, semid;

    /* connect to shm */
    mbctrl = (mb_ctrl_t *)shm_connect(SD_MEM_NAME);
    if (mbctrl == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"shm_connect: %s error %d %s\n", SD_MEM_NAME, errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    if (mbctrl->req_que == 0 && mbctrl->rsp_que == 0)
    {
    	SysLog(SYS_TRACE_ERROR," sys err ");
        tux_errno = SYS_OS;
        return -1;
    }

    tSvrTab   = (server_tab_t *)&mbctrl[1];
    tSvrPid   = (server_pid_t *)&tSvrTab[mbctrl->nsvrnum];
    tSvcTab   = (service_tab_t *)&tSvrPid[mbctrl->npidnum];

    /* connect to sem */
    mbctrl->semid = sem_connect(SD_SEM_NAME, 1);
    if (mbctrl->semid < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"sem_connect: %s error %d %s\n", SD_SEM_NAME, errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }
    
    rc = que_connect();
    if (rc < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"que_connect error \n");
        tux_errno = SYS_OS;
        return -1;
    }

    return 0;
}

int tux_locatemb(const char *svr_name)
{
    int i;

    for (i = 0; i < sdNumberOfSvr(); ++i) 
    {
        if (!strncmp(svr_name, sdGetSvrSvcName(i), TUX_NAME_LEN))
            return i;
    }
    
    return -1;
}

int tux_get_timeout(const char *svr_name)
{
    char buf[512 + 1];
    FILE *fpcfg;
    char *p;
    char name[TUX_NAME_LEN + 1];
    int  timeout = MAX_TIMEOUT_SEC;

    fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG,
                 errno, strerror(errno));
        return timeout;
    }

    while (1) 
    {
        memset(buf, 0, sizeof(buf));
        if (cf_nextparm(fpcfg, "sd.svc", buf) < 0)
            break;

        p = strtok(buf, " ");
        if (p == NULL)
            continue;
        memset(name, 0, sizeof(name));
        strncpy(name, p, TUX_NAME_LEN);

        if (strncmp(name, svr_name, TUX_NAME_LEN))
            continue;

        p = strtok(NULL, " ");
        if (p == NULL)
            break;

        timeout = atoi(p);
        break;
    }

    cf_close(fpcfg);
    return timeout;
}


long tux_real_len(const char *bufptr)
{
    struct tpmemory *p, *p2;

    if (bufptr == NULL) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    if (!IS_MAGIC(bufptr)) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    p  = (struct tpmemory *)bufptr - 1;
    p2 = p + ALLOC_UNITS(p->len) - 1;

    if (p->len != p2->len || p->magic != p2->magic ||
        p->type != p2->type || p->subtype != p2->subtype) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    /* STRING */
    if (!strcmp(p->type, stringtype))
        return (strlen(bufptr) + 1);

    /* CARRAY */
    return (p->len);
}

void list_server()
{
    int  i, taskid, req_queid, rsp_queid;

    printf("SvrId  ProgName        ServiceName     Pid_Tot     Status  \n");
    printf("-----------------------------------------------------------------------------\n");
    for (i = 0; i < sdNumberOfSvr(); ++i) 
    {
        if (sdGetSvrId(i) != -1) 
        {
            printf("%-4d   %-16s %-16s %-8d   %-16s\n",
                   sdGetSvrId(i), sdGetSvrName(i),
                   sdGetSvrSvcName(i), sdGetSvrNum(i),
                   (sdGetSvrStat(i) == TUX_IDLE ? "idle" : (sdGetSvrStat(i) == TUX_DEAD ? "dead" : "avail")));
        }
    }
}

void list_service()
{
    int i;
    int j = 0;
    int task_id;
    unsigned int req_tot = 0;
    

    printf("SvrId  ServiceName        PID     RqDone  Die_Count \n");
    printf("-----------------------------------------------------------------------\n");
    for (i = 0; i < sdNumberOfPid(); ++i) 
    {
        if (sdGetSvrPid(i) != -1) 
        {
        	task_id = sdGetSvrPidPos(i);
        	req_tot = sdGetSvrPidDone(i);       	
            printf("%-4d   %-16s  %-6d    %-8u  %-8d\n",
                   sdGetSvrPidId(i), sdGetSvrSvcName(task_id), sdGetSvrPid(i), req_tot, sdGetSvrPidRestartNum(i));
        }
    }

}

void list_queue()
{
    int i;
    int req_num = 0, rsq_num = 0;

    
    req_num = MsqCount(sdReqQueId());
    rsq_num = MsqCount(sdRspQueId());
    
    printf("que_id            msg_num  \n");
    printf("-----------------------------------------------------------------------\n");
    printf("%-8u              %-8d \n",sdReqQueId(),req_num);
    printf("%-8u              %-8d \n",sdRspQueId(),rsq_num);
    
    return ;
}

int tux_deletesvc(int taskid)
{
    int i;

    SysLog(SYS_TRACE_NORMAL,"mb_deletemb_local_byid: [%d]..........", sdGetSvrId(taskid));

    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    if (tux_lock()) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_lock error %d %s", errno, strerror(errno));
        tux_errno = SYS_OS;
        return -1;
    }

    for (i = 1; i < sdNumberOfSvr(); ++i) 
    {
        if (sdGetSvrId(i) == taskid)
            sdGetSvrStat(i) = TUX_UNAVAIL;
    }

    tux_unlock();
    return 0;
}

int tux_find_service(const char *svr_name, int *pos)
{
    int i;
    
    if (svr_name == NULL)
    {
    	return -1;
    }

    for (i = 0; i < sdNumberOfSvr(); i++) 
    {
        if (sdGetSvcFlag(i) == -1 && *pos < 0 ) 
        {
        	*pos = i;	
        	continue;
        }
        
        if (!strncmp(sdGetSvcName(i), svr_name, TUX_NAME_LEN))
            return i;
    }
    
    return -1;
}

static int tux_new_service(const char *svr_name, int (*sd_fun)(sd_info_t *))
{
    int rc, pos;
    pos = -1;

    /* first search the service */
    rc = tux_find_service(svr_name, &pos);
    if (rc >= 0) 
    {
        SysLog(SYS_TRACE_NORMAL,"service %s has been advertise", svr_name);
        if (tSvcTab[rc].sd_fun != sd_fun) 
        {
            tux_errno = SYS_EMATCH;
            return -1;
        }
        return rc;
    }

    memset(&tSvcTab[pos], 0, sizeof(service_tab_t));
    strncpy(tSvcTab[pos].name, svr_name, TUX_NAME_LEN);
    tSvcTab[pos].sd_fun = sd_fun;
    tSvcTab[pos].flags == SVC_INUSE;
    
    SysLog(SYS_TRACE_NORMAL,"service [%s] [%d]", svr_name, pos );
    return pos;
}

static int tux_del_service(const char *svr_name)
{
    int rc;
    int pos = -1;

    /* first search the service */
    rc = tux_find_service(svr_name, &pos);
    if (rc < 0) {
        SysLog(SYS_TRACE_ERROR,"service %s not found", svr_name);
        tux_errno = SYS_NOENT;
        return -1;
    }

    memset(&tSvcTab[rc], 0, sizeof(service_tab_t));
    return 0;
}

const char* tux_strerror(int err)
{
    switch (err) {
    case SYS_LIMIT:
        return "system resource limit over";
    case SYS_INVAL:
        return "input parameter invalid";
    case SYS_OS:
        return "operator system error";
    case SYS_NOENT:
        return "service not found";
    case SYS_TIMEOUT:
        return "transaction timeout";
    case SYS_PROTO:
        return "protocol error";
    case SYS_SVCFAIL:
        return "service failure";
    case SYS_EMATCH:
        return "service match error";
    case SYS_GOTSIG:
        return "receive signal exit";
    case SYS_SYSTEM:
        return "system error";
    case SYS_ERR_SVC:
        return "serviece not found";
    case SYS_DEAD_SVC:
        return "serviece dead";
    default:
        return "unknown error";
    }
}

int find_server_id(int svrid)
{
    int i;

    for (i = 1; i < sdNumberOfSvr(); i++)
        if (sdGetSvrId(i) == svrid)
            return i;
    return -1;
}


int find_svrno_by_service(const char *svr_name)
{
    int i;

    for (i = 0; i < sdNumberOfSvr(); i++)
    {
    	if (sdGetSvrId(i) != -1)
    	{
    		SysLog(SYS_TRACE_NORMAL,"servername: %-15s", sdGetSvrSvcName(i));
    	}
    	
    	if (!strncmp(sdGetSvrSvcName(i), svr_name, TUX_NAME_LEN))
    		return i;
    }

    return -1;
}

int tux_advertise(const char *svr_name, int (*sd_fun)(sd_info_t *))
{
    int rc, pos, timeout, svrno;

    if (svr_name == NULL || sd_fun == NULL) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }
    
    /* check system attach */
    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"sys ipc is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }
    
    svrno = find_svrno_by_service(svr_name);
    if (svrno < 0)
    {
    	SysLog(SYS_TRACE_ERROR,"find_svrno_by_service err [%s]", svr_name);
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    timeout = sdGetSvrTimeOut(svrno);
    SysLog(SYS_TRACE_NORMAL," service %s timeout %d", svr_name, timeout);

    /* save mb_name, mb_fun */
    pos = tux_new_service(svr_name, sd_fun);
    if (pos < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_new_service %s error %d", svr_name, tux_errno);
        return pos;
    }

    /* save the service svrid */
    tSvcTab[pos].svrid = svrno;
    return 0;
}

int tux_unadvertise(const char *svr_name)
{
    int rc;

    /* check parameter valid */
    if (svr_name == NULL) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    /* check system attach */
    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"ipc is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    /* delete service */
    rc = tux_del_service(svr_name);
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_del_service %s error %d", svr_name, tux_errno);
        return rc;
    }

    return 0;
}


static int tux_return_reply(int rval, long rcode, const char *rsp, long rsplen, long flags)
{
    int rc;
    IPCQueMsg    ipcMsgOut={0};
    IPCChkMsq    ipcChkMsq={0};
    
    /* 判断应答标志 -1 - 不应答 */
    if (flags == -1)
    {
    	return 0;
    }
   
    if (rsp != NULL) 
    {
        if (rsplen == 0) 
        {
            rsplen = tux_real_len(rsp);
        }
    }
    else
    {
    	rsplen = 0;
    	return 0;
    }  

    if (rsplen < 0 || rsplen > mbctrl->max_msg_len) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }
    
    gipcMsgHead.len = (int)rsplen;
    ipcMsgOut.nMsgType =  gipcMsgHead.req_msg_type;
    ipcMsgOut.nMsgSource = 0;
    memcpy((char *)&ipcMsgOut.sBuf, (char*)&gipcMsgHead, sizeof(gipcMsgHead));
    memcpy((char *)&ipcMsgOut.sBuf+sizeof(gipcMsgHead), rsp, gipcMsgHead.len);
    ipcChkMsq.nChkFlag = MSQ_CHK_YES;
    rc = MsqSnd(mbctrl->rsp_que_id, &ipcMsgOut, sizeof(gipcMsgHead) + gipcMsgHead.len, &ipcChkMsq);
	if (rc != 0)
	{
		SysLog(SYS_TRACE_ERROR, "msgsnd err error %d, %s ", errno, strerror(errno));
		return -1;
	}

    return 0;
}

void tux_return(int rval, long rcode, const char *rsp, long rsplen, long flags)
{
    int rc;

    /* exception call */
    if (_mb_longjmp == 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_return exception call, exit");
        tux_free((char*)rsp);
        exit(1);
    }

    /* set rval TUX_FAIL or TUX_SUCCESS */
    rval = ((rval != TUX_FAIL && rval != TUX_SUCCESS) ? TUX_FAIL : rval);

    /* check system attach */
    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");        
        tux_errno = SYS_SYSTEM;
        longjmp(_mb_jmpbuf, SYS_SYSTEM);
    }

    /* return reply */
    rc = tux_return_reply(rval, rcode, rsp, rsplen, flags);
    if (rc) 
    {
        longjmp(_mb_jmpbuf, rc);
    }

    longjmp(_mb_jmpbuf, 1);
}

int tux_acall(const char *svr_name, const char *req_buf, long req_buflen, long flags)
{
    int rc, svrno,timeout;
    int iMsgLen;
    IPCQueMsg      ipcMsgIn={0};
    IPCQueMsg      ipcMsgOut={0};
    IPCChkMsq      ipcChkMsq={0};

    /* check input parameters */
    if (svr_name == NULL || req_buf == NULL) 
   {
        tux_errno = SYS_INVAL;
        return -1;
    }

    if (req_buflen == 0) 
    {
        req_buflen = tux_real_len(req_buf);
    }
    
    if (req_buflen < 0 || req_buflen > mbctrl->max_msg_len) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    /* locate mb_name */
    svrno = tux_locatemb(svr_name);
    if (svrno < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_locatemb %s error", svr_name);
        tux_errno = SYS_ERR_SVC;
        return -1;
    }
    
    if (sdGetSvrStat(svrno) == TUX_UNAVAIL || sdGetSvrStat(svrno) == TUX_DEAD)
    {
    	SysLog(SYS_TRACE_ERROR,"service %s dead", svr_name);
        tux_errno = SYS_DEAD_SVC;
        return -1;
    }

    timeout = sdGetSvrTimeOut(svrno);
    ipcMsgIn.nMsgType = sdGetSvrMsgType(svrno);
    ipcMsgIn.nMsgSource = 0;
    memcpy(ipcMsgIn.sBuf, req_buf, req_buflen);
    
    ipcChkMsq.nChkFlag = MSQ_CHK_YES;    
    rc = MsqSnd(mbctrl->req_que_id, &ipcMsgIn, req_buflen, &ipcChkMsq);
	if (rc != 0)
	{
		tux_errno = SYS_OS;
		SysLog(SYS_TRACE_ERROR, "msgsnd err error %d, %s ", errno, strerror(errno));
		return -1;
	
    }
    
    return 0;
}


int tux_getreply(int cd, char *rsp_buf, long *rsp_len, long flags)
{
    int rc;
    int iMsgLen;
    int iRcvMod;
    long msg_type = cd;
    IPCQueMsg      ipcMsgIn={0};
    IPCChkMsq      ipcChkMsq={0};

    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }
    
    iRcvMod = (flags == 0) ? MSQ_MODE_BLOCK : MSQ_MODE_NONBLOCK;
    iMsgLen = MsqRcv(mbctrl->rsp_que_id, &ipcMsgIn, BUS_MAX_MSG_LEN, msg_type, iRcvMod);
    if (iMsgLen <= 0) 
    {
    	tux_errno = errno;
    	return -1;
    }
    
    memcpy(rsp_buf, ipcMsgIn.sBuf, iMsgLen);
    *rsp_len = iMsgLen;	
	return 0;
}

int tux_call(const char *svr_name, const char *req_buf,long req_buflen,char *rsp_buf, long *rsp_buflen,long flags)
{
    int rc, svrno,timeout;
    int iMsgLen;
    IPCQueMsg      ipcMsgIn={0};
    IPCQueMsg      ipcMsgOut={0};
    IPCChkMsq      ipcChkMsq={0};

    /* check input parameters */
    if (svr_name == NULL || req_buf == NULL || rsp_buf == NULL || rsp_buflen == NULL) 
   {
        tux_errno = SYS_INVAL;
        return -1;
    }

    if (req_buflen == 0) 
    {
        req_buflen = tux_real_len(req_buf);
    }
    
    if (req_buflen < 0 || req_buflen > mbctrl->max_msg_len) 
    {
        tux_errno = SYS_INVAL;
        return -1;
    }

    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }

    /* locate mb_name */
    svrno = tux_locatemb(svr_name);
    if (svrno < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_locatemb %s error", svr_name);
        tux_errno = SYS_NOENT;
        return -1;
    }
    
    if (sdGetSvrStat(svrno) == TUX_UNAVAIL || sdGetSvrStat(svrno) == TUX_DEAD)
    {
    	SysLog(SYS_TRACE_ERROR,"service %s dead", svr_name);
        tux_errno = SYS_DEAD_SVC;
        return -1;
    }

    /* get transaction timeout seconds */
    timeout = sdGetSvrTimeOut(svrno);
    ipcMsgIn.nMsgType = sdGetSvrMsgType(svrno);
    ipcMsgIn.nMsgSource = 0;
    memcpy(ipcMsgIn.sBuf, req_buf, req_buflen);
    
    ipcChkMsq.nChkFlag = MSQ_CHK_YES;    
    rc = MsqSnd(mbctrl->req_que_id, &ipcMsgIn, req_buflen, &ipcChkMsq);
	if (rc != 0)
	{
		SysLog(SYS_TRACE_ERROR, "msgsnd err error %d, %s ", errno, strerror(errno));
		return -1;
	}
	
	SysLog(SYS_TRACE_ERROR, "send one request to message ok");
    /* recv response */
    tux_settimer(timeout);
    iMsgLen = MsqRcv(mbctrl->rsp_que_id, &ipcMsgOut, BUS_MAX_MSG_LEN, gipcMsgHead.req_msg_type, MSQ_MODE_BLOCK);
    if (iMsgLen <= 0) 
    {
    	SysLog(SYS_TRACE_ERROR,"MsqRcv queue s error: %d, %s ", errno, strerror(errno));
    	tux_unsettimer();
    	return -1;
    }
    tux_unsettimer();
    
    SysLog(SYS_TRACE_ERROR, "MsqRcv one reponse to message ok");

    if (tux_timer_expired()) 
    {
        tux_errno = SYS_TIMEOUT;
        return -1;
    }

    memcpy(rsp_buf, ipcMsgOut.sBuf, iMsgLen);
    *rsp_buflen = iMsgLen;

    return 0;
}

int tux_forward(const char *svr_name, const char *data, long len, long flags)
{
    int rc, svrno;
    IPCQueMsg    ipcMsgIn={0};
    IPCChkMsq   ipcChkMsq={0};

    /* exception call */
    if (_mb_longjmp == 0) 
    {
    	if (data != NULL)
            tux_free((char*)data);
       SysLog(SYS_TRACE_ERROR,"tux_forward exception call, exit");
        exit(1);
    }

    /* check input parameters */
    if (svr_name == NULL || data == NULL) 
    {
        tux_errno = SYS_INVAL;
        longjmp(_mb_jmpbuf, -1);
    }

    if (len == 0) 
    {
        len = tux_real_len(data);
    }

    if (len < 0 || len > mbctrl->max_msg_len) 
    {
        tux_errno = SYS_INVAL;
        longjmp(_mb_jmpbuf, -1);
    }

    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        longjmp(_mb_jmpbuf, -1);
    }

    /* locate mb_name */
    svrno = tux_locatemb(svr_name);
    if (svrno < 0) {
        SysLog(SYS_TRACE_ERROR,"tux_locatemb %s error", svr_name);
        tux_errno = SYS_NOENT;
        longjmp(_mb_jmpbuf, -1);
    }
    
    ipcMsgIn.nMsgType = sdGetSvrMsgType(svrno);
    ipcMsgIn.nMsgSource = 0;
    memcpy(ipcMsgIn.sBuf, data, len);
    ipcChkMsq.nChkFlag = MSQ_CHK_YES;    
    rc = MsqSnd(mbctrl->req_que_id, &ipcMsgIn, len, &ipcChkMsq);
	if (rc != 0)
	{
		SysLog(SYS_TRACE_ERROR, "msgsnd err error %d, %s ", errno, strerror(errno));
		return -1;
	}

    longjmp(_mb_jmpbuf, 1);
}

int tux_service(int svrid, int svc_pos)
{
    int rc, service_index;
    int pos =1;
    int iMsgLen;
    long MsgType = svrid+SVR_MSG_TYPE_ROOT;
    IPCQueMsg    ipcMsgIn={0};
    sd_info_t    sdInfo={0};

    /* check system attach */
    if (!sdIsAttach()) 
    {
        SysLog(SYS_TRACE_ERROR,"message queue system is down");
        tux_errno = SYS_SYSTEM;
        return -1;
    }
    
    while (1)
    {
    	SysLog(SYS_TRACE_ERROR,"test_11111 ");
    	/* receive packet */
        iMsgLen = MsqRcv(mbctrl->req_que_id, &ipcMsgIn, BUS_MAX_MSG_LEN, MsgType, MSQ_MODE_BLOCK);
        if(iMsgLen <= 0)
        {
        	if (errno ==  EINTR)
        	{
        		continue;
        	}
        	
        	SysLog(SYS_TRACE_ERROR,"MsqRcv from  error [%d] [%s]", errno , strerror(errno));
            return -1;
        }
        
        break;
    }
    
    memset(&gipcMsgHead, 0, sizeof(gipcMsgHead));
    memcpy((char *)&gipcMsgHead, (char *)ipcMsgIn.sBuf, sizeof(gipcMsgHead));
    
    SysLog(SYS_TRACE_ERROR,"test_2222 ");
    
    service_index = tux_find_service(gipcMsgHead.name, &pos);
    if (service_index < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_find_service %s error", gipcMsgHead.name);
        return 0;
    }
    
    strncpy(sdInfo.name, gipcMsgHead.name, TUX_NAME_LEN);
    sdInfo.len = gipcMsgHead.len;
    sdInfo.data = tux_alloc("CARRAY", 0, sdInfo.len);
    memcpy(sdInfo.data, ipcMsgIn.sBuf+sizeof(gipcMsgHead), gipcMsgHead.len);
    sdInfo.data[gipcMsgHead.len] = 0x00;
    
    SysLog(SYS_TRACE_ERROR,"test_3333");
    
    sdGetSvrPidDone(svc_pos)++;
    _mb_longjmp = 1;
    rc = setjmp(_mb_jmpbuf);
    if (rc == 0) 
    {
        tSvcTab[service_index].sd_fun(&sdInfo);
    } 
    else if (rc < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_return error %d", tux_errno);
    }
    
    SysLog(SYS_TRACE_ERROR,"test_4444");

    tux_free(sdInfo.data);
    _mb_longjmp = 0;
    return 0;
}

int clean_task_msg(long msg_type)
{
	IPCQueMsg msgbuf;
	
	while (1)
	{
		if (MsqRcv (mbctrl->req_que_id, &msgbuf, BUS_MAX_MSG_LEN , msg_type, MSQ_MODE_NONBLOCK) < 0)
			return 0;	
	}
	
	return (0); /* success */
}

/******************************************** 文件结束 ******************************************/

