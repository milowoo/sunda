/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能: 系统服务总控文件
 *
 *  Edit History:
 *
 *    2012/03/09 - MiloWoo 建立.
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "sunda.h"
#include "CfgOpr.h"
#include "trace.h"
#include "task.h"

int reload_server_param(int taskid);

static void daemonize(void)
{
    sigset(SIGTTOU, SIG_IGN);
    sigset(SIGTTIN, SIG_IGN);
    sigset(SIGTSTP, SIG_IGN);

    if (0 != fork()) exit(0);
    if (-1 == setsid()) exit(0);
    sigset(SIGHUP, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    if (0 != fork()) exit(0);
}

static int server_find_pid(long pid)
{
    int i;

    for (i = 1; i < sdNumberOfPid(); i++)
        if (sdGetSvrPid(i) == pid)
            return i;
    return -1;
}

static int server_get_pidentry()
{
    int i;

    for (i = 1; i < sdNumberOfPid(); i++)
        if (sdGetSvrPid(i) == -1)
        	  return i;
    return -1;
}

static int server_get_identry()
{
    int i;

    for (i = 1; i < sdNumberOfSvr(); i++)
        if (sdGetSvrId(i) == -1)
            return i;
    return -1;
}

static int service_get_pidentry()
{
    int i;

    for (i = 1; i < sdNumberOfSvr(); i++)
        if (sdGetSvcFlag(i) == -1)
        	  return i;
    return -1;
}

static int str_split(const char *src, const char *splitstr, char *buf, int buf_size, int n)
{
    char  temp[2048 + 1];
    int   pos;
    char  *p;

    pos = 1;
    strcpy(temp, src);
    p = strtok(temp, splitstr);
    while (p) {
        if (pos == n) {
            strncpy(buf, p, buf_size);
            buf[buf_size] = 0x00;
            return (p - temp);
        }
        p = strtok(NULL, splitstr);
        pos++;
    }
    return -1;
}

static int register_server(int svrnum, char *buf)
{
    int   rc;
    int   pos = -1;
    int   svcnum =0;
    int   service_pos;
    char  *p = buf, *q;
    char  temp[128 + 1];
    char  sFullName[128+1]={0};
    char  sSvrName[TUX_NAME_LEN+1]={0};
    char  sSvcName[TUX_NAME_LEN+1]={0};

    p = strtok(buf, " ");  
    strncpy(sSvrName, p, TUX_NAME_LEN);
    AllTrim(sSvrName); 
    sprintf(sFullName,  "%s/bin/%s", getenv("HOME"), sSvrName);
    rc = access(sFullName, F_OK);
	if (rc != 0)
	{
	    SysLog(SYS_TRACE_ERROR, " the %s exec file no found ", sSvrName);
	    return 1;
	}
	
	memset(sdGetSvrName(svrnum), 0, TUX_NAME_LEN + 1);
    memset(sdGetSvrSvcName(svrnum), 0, TUX_NAME_LEN + 1);
    memset(sdGetSvrParam(svrnum), 0, SERVER_MAX_PARAM + 1);
    
    strcpy(sdGetSvrName(svrnum), sSvrName);
    sdGetSvrNum(svrnum) = 5;
    sdGetSvrStat(svrnum)  = SVR_PERM;
    sdGetSvrRspFlag(svrnum) = SVR_REPLY_NEED;  
    sdGetSvrTimeOut(svrnum) = MAX_TIMEOUT_SEC;

    p = strtok(NULL, " ");
    while (1) 
    {
        memset(temp, 0, sizeof(temp));
        rc = str_split(p, "= ", temp, sizeof(temp) - 1, 1);
        if (rc < 0) break;
        p += rc + strlen(temp) + 1;

        if (!strcmp(temp, "SRVID")) {  /* server id */
            memset(temp, 0, sizeof(temp));
            rc = str_split(p, "= ", temp, sizeof(temp) - 1, 1);
            if (rc < 0) break;
            sdGetSvrId(svrnum) = atoi(temp);
            sdGetSvrMsgType(svrnum) = SVR_MSG_TYPE_ROOT + sdGetSvrId(svrnum);     
            p += rc + strlen(temp) + 1;
        } else if (!strcmp(temp, "SRVNUM")) {  /* server process number */
            memset(temp, 0, sizeof(temp));
            rc = str_split(p, "= ", temp, sizeof(temp) - 1, 1);
            if (rc < 0) break;
            sdGetSvrNum(svrnum) = atoi(temp);
            p += rc + strlen(temp) + 1;
        } 
        else if (!strcmp(temp, "RQADDR")) { /* server request address */
            memset(temp, 0, sizeof(temp));
            rc = str_split(p, "= ", temp, sizeof(temp) - 1, 1);
            if (rc < 0) break;
            strncpy(sSvcName, temp, TUX_NAME_LEN);
            AllTrim(sSvcName);
            strcpy(sdGetSvrSvcName(svrnum), sSvcName);      
            p += rc + strlen(temp) + 1;
        } else if (!strcmp(temp, "REPLYQ")) { /* server reply queue flag */
            memset(temp, 0, sizeof(temp));
            rc = str_split(p, "= ", temp, sizeof(temp) - 1, 1);
            if (rc < 0) break;
            	
            if (temp[0] == 'Y' || temp[0] == 'y')
                sdGetSvrRspFlag(svrnum) = SVR_REPLY_NEED;
            else
                sdGetSvrRspFlag(svrnum) = SVR_REPLY_NONEED;
            p += rc + strlen(temp) + 1;
        } else if (!strcmp(temp, "SRVPARM")) { /* server parameter */
            p = strchr(p, '"');
            if (p == NULL)
                return -1;
            q = strchr(p + 1, '"');
            if (q == NULL)
                return -2;
                
            strncpy(sdGetSvrParam(svrnum), p + 1, (int)(q - p - 1));
            p = q + 1;
        }
    }

    return 0;
}

int server_start_pid(int taskid, int pid_index)
{
    int   rc;
    pid_t pid;
    char  param[128 + 1]={0};

    /* task parameter: taskid seq ... */
    sprintf(param, "%d  %d  ", pid_index, sdGetSvrId(taskid));
    strcat(param, sdGetSvrParam(taskid));

    SysLog(SYS_TRACE_NORMAL,"start task: id %d [%s] [%s]", sdGetSvrId(taskid), sdGetSvrName(taskid), param);

    /* start server */
    pid = server_load(sdGetSvrName(taskid), param);
    if (pid < 0) 
    {
        SysLog(SYS_TRACE_ERROR,"server_load %s %s error %d %s", sdGetSvrName(taskid), sdGetSvrParam(taskid), errno, strerror(errno));
        return -1;
    }

    sdGetSvrPid(pid_index) = pid;                /* pid */
    sdGetSvrPidStarTime(pid_index) = time(NULL); /* task start time */
    return 0;
}

int server_run(int srvnum)
{
    int i, rc, pid_index;

    sdGetSvrType(srvnum) = SVR_PERM;
    for (i = 0; i < sdGetSvrNum(srvnum); ++i) 
    {
        /* get pid entry */
        pid_index = server_get_pidentry();
        if (pid_index < 0) 
        {
            SysLog(SYS_TRACE_ERROR,"system task process number over limit");
            return SYS_LIMIT;
        }

        SysLog(SYS_TRACE_NORMAL,"task id %d seq %d pid entry %d", sdGetSvrId(srvnum), i + 1, pid_index);
        
        sdGetSvrPidId(pid_index) = sdGetSvrId(srvnum);
        sdGetSvrPidSeq(pid_index) = i;
        sdGetSvrPidPos(pid_index) = srvnum;
        rc = server_start_pid(srvnum, pid_index);
        if (rc) 
        {
            sdSvrPidClear(pid_index);
            return rc;
        }
        
        sdGetSvrStat(srvnum) = TUX_IDLE;
    }

    return 0;
}

int server_kill(int taskid, int clear)
{
    int   rc, i;
    pid_t pid;

    for (i = 0; i < sdNumberOfPid(); i++) 
    {
        if (sdGetSvrPidId(i) == sdGetSvrId(taskid) && sdGetSvrPidId(i) != -1 && sdGetSvrPidId(i) != TUX_DEAD) 
        {
            rc = kill(sdGetSvrPid(i), SIGTERM);
            if (rc == -1)
                SysLog(SYS_TRACE_ERROR,"kill task: id %d %s pid %ld, error: %d, %s.", sdGetSvrId(taskid), sdGetSvrName(taskid),
                             sdGetSvrPid(i), errno, strerror(errno));
            else
                SysLog(SYS_TRACE_ERROR,"kill task: id %d %s pid: %ld succeed.",
                         sdGetSvrId(taskid), sdGetSvrName(taskid), sdGetSvrPid(i));

		    if (clear) sdSvrPidClear(i);
        }
    }
    return 0;
}

void unload_server()
{
    int i;

    if (!sdIsAttach())
        return;

    sigset(SIGCLD, SIG_IGN);

    for (i = 1; i < sdNumberOfSvr(); i++) 
    {
        if (sdGetSvrId(i) != -1) 
        {
            server_kill(i, 1);
            sbSvrTabClear(i);
            SysLog(SYS_TRACE_DEBUG,"task %d kill", sdGetSvrId(i));
        }
    }
}

void load_server()
{
    int i, rc;

    for (i = 1; i < sdNumberOfSvr(); i++) 
    {
        if (sdGetSvrId(i) != -1)
        {
            server_run(i);
        }
    }
}

void server_deatch(int sig)
{
    int pid_index, rc, taskid;
    int status;
    pid_t pr;
    int time_out=1;
    char sSvrName[TUX_NAME_LEN+1]={0};

    sighold(SIGTERM);
    sighold(SIGCLD);
    SysLog(SYS_TRACE_NORMAL,"server_deatch catch signal: %d.", sig);

    while ((pr = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) 
    {
        SysLog(SYS_TRACE_NORMAL,"pid %ld exit", pr);
        pid_index = server_find_pid(pr);
        if (pid_index < 0)
            continue;

        taskid = sdGetSvrPidPos(pid_index);
        
        SysLog(SYS_TRACE_NORMAL,"task id %d, one server exit.", sdGetSvrPidId(taskid));

        sdGetSvrNum(taskid)--;
        sdGetSvrPidLastDie(taskid) = time(NULL);
        sdGetSvrPidRestartNum(taskid)++;
        
        if (sdGetSvrPidRestartNum(pid_index) > 50)
        {
        	sdGetSvrType(taskid) = SVR_TEMP;
        }
        
        if (sdGetSvrType(taskid) != SVR_TEMP)
        {
        	sdGetSvrPidRestartNum(pid_index)++;

            SysLog(SYS_TRACE_NORMAL,"try start task: id %d %s, grace retry count: %d",
                    sdGetSvrPidId(taskid), sdGetSvrName(taskid), sdGetSvrPidRestartNum(pid_index));
            
            reload_server_param(taskid);
            
            memset(sSvrName, 0, sizeof(sSvrName));
            strncpy(sSvrName, sdGetSvrSvcName(taskid), TUX_NAME_LEN);
            time_out = tux_get_timeout(sSvrName);
            sdGetSvrTimeOut(taskid) = time_out; 
            rc = server_start_pid(taskid, pid_index);
            if (rc != 0) 
            {
                SysLog(SYS_TRACE_ERROR,"server_start_pid task: id %d %s error %d", sdGetSvrPidId(taskid), sdGetSvrName(taskid), rc);
            }
            else
            {
		    	sdGetSvrStat(pid_index) = TUX_AVAIL;
		    	sdGetSvrPidStarTime(pid_index) = time(NULL);
		    }
        }
        
        if (sdGetSvrNum(taskid) == 0) 
        {
            tux_deletesvc(taskid);
        }
    }
    
    sigrelse(SIGCLD);
    sigset(SIGCLD, server_deatch);
    sigrelse(SIGTERM);
}

int reload_server_param(int taskid)
{
    int  i, rc;
    char buf[512 + 1]={0};
    FILE *fpcfg;

    fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG, errno, strerror(errno));
        return -1;
    }

    for (i = 1; i < sdNumberOfSvr(); i++) 
    {
        memset(buf, 0, sizeof(buf));
        if (cf_nextparm(fpcfg, "tsk.task", buf) < 0)
            break;

        if (sdGetSvrId(i) != sdGetSvrId(taskid))
        {
            continue;
        }

        rc = register_server(taskid, buf);
        if (rc <0) 
        {
            SysLog(SYS_TRACE_ERROR,"register_server error: %d", rc);
            cf_close(fpcfg);
            return -1;
        }
        if (rc == 1)
        {
        	break;
        }

        SysLog(SYS_TRACE_NORMAL,"id: %4d, svrname: %-16s, svcname: %-16s, pid_tot: %4d, in_param: %-24s",
           sdGetSvrId(i) , sdGetSvrName(i), sdGetSvrSvcName(i),  sdGetSvrNum(i), sdGetSvrParam(i));

        break;
    }

    cf_close(fpcfg);
    return 0;
}

int server_load_cfg()
{
    int  i, rc, time_out;
    char buf[512 + 1];
    char sSvrName[TUX_NAME_LEN + 1]={0};
    FILE *fpcfg;

    fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG, errno, strerror(errno));
        return -1;
    }

    for (i = 1; i < sdNumberOfSvr(); i++) 
    {
        memset(buf, 0, sizeof(buf));
        if (cf_nextparm(fpcfg, "tsk.task", buf) < 0)
            break;

        rc = register_server(i, buf);
        if (rc < 0) 
        {
            SysLog(SYS_TRACE_ERROR,"register_server error: %d", rc);
            cf_close(fpcfg);
            return -1;
        }
        
        if (rc > 0)
        {
        	i--;
        }
    }
    
    cf_close(fpcfg);
    
    //取超时设定时间
    for (i = 1; i < sdNumberOfSvr(); i++) 
    {
    	if (sdGetSvrId(i) == -1)
    	{
    		continue;
    	}
    	
    	memset(sSvrName, 0, sizeof(sSvrName));
        strncpy(sSvrName, sdGetSvrSvcName(i), TUX_NAME_LEN);
        time_out = tux_get_timeout(sSvrName);
        sdGetSvrTimeOut(i) = time_out; 
    }
    
    for (i = 0; i < sdNumberOfSvr(); i++)
        if (sdGetSvrId(i) != -1)
            SysLog(SYS_TRACE_NORMAL,"id: %4d, servername: %-15s, servicename:%-15s servernum: %2d, taskparam: %-16s",
                    sdGetSvrId(i) , sdGetSvrName(i), sdGetSvrSvcName(i), sdGetSvrNum(i), sdGetSvrParam(i));

    
    return 0;
}

int run_task_id(int taskid, char *sTuxName)
{
	int   rc, time_out =0;
	char  *p = NULL;
    char  temp[512+1]={0};
    char  buf[512+1]={0};
    char  sFullName[128+1]={0};
    char  sSvrName[TUX_NAME_LEN+1]={0};
    FILE *fpcfg;
    
    fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG, errno, strerror(errno));
        sbSvrTabClear(taskid);
        return SYS_SYSTEM;
    }
    
    while (1) 
    {
        memset(temp, 0, sizeof(temp));
        memset(buf,  0, sizeof(buf));
        if (cf_nextparm(fpcfg, "tsk.task", temp) < 0) 
        {
            break;
        }
        
        strcpy(buf, temp); 
        p = strtok(temp, " ");  
        strncpy(sSvrName,  p, TUX_NAME_LEN);
        AllTrim(sSvrName);      
        if (strncmp(sTuxName, sSvrName, TUX_NAME_LEN) != 0)
        	continue;

        rc = register_server(taskid, buf);
        if (rc) 
        {
            sbSvrTabClear(taskid);
            SysLog(SYS_TRACE_ERROR,"register_server error: %d", rc);
            cf_close(fpcfg);
            return  SYS_SYSTEM;
        }
    
        rc = server_run(taskid);
        if (rc) 
        {
            sbSvrTabClear(taskid);
            SysLog(SYS_TRACE_ERROR,"server_run error %d", rc);
            cf_close(fpcfg);
            return  SYS_OS;
        }
    }
    cf_close(fpcfg);
    
    time_out = tux_get_timeout(sTuxName);
    sdGetSvrTimeOut(taskid) = time_out;
    sdGetSvrStat(taskid) = TUX_IDLE; 
    
    return 0;
}

int run_one_task(int taskid, int tux_id)
{
	int rc;
	char buf[512+1]={0};
	FILE *fpcfg;
	
	fpcfg = cf_open(SYS_PARAM_CFG);
    if (fpcfg == NULL) 
    {
        SysLog(SYS_TRACE_ERROR,"cf_open %s error %d %s", SYS_PARAM_CFG, errno, strerror(errno));
        sbSvrTabClear(taskid);
        return  SYS_SYSTEM;
    }
    
    while (1) 
    {
         memset(buf, 0, sizeof(buf));
         if (cf_nextparm(fpcfg, "tsk.task", buf) < 0) 
         {
             sbSvrTabClear(taskid);
             SysLog(SYS_TRACE_ERROR,"id %d not found in system cfg", tux_id);
             cf_close(fpcfg);
             return SYS_NOENT;
         }
    
         rc = register_server(taskid, buf);
         if (rc) 
         {
             sbSvrTabClear(taskid);
             SysLog(SYS_TRACE_ERROR,"register_server error: %d", rc);
             cf_close(fpcfg);
             return SYS_SYSTEM;
        }
    
        if (sdGetSvrId(taskid) == tux_id) 
        {
            rc = server_run(taskid);
            if (rc) 
            {
                sbSvrTabClear(taskid);
                SysLog(SYS_TRACE_ERROR,"server_run error %d", rc);
                cf_close(fpcfg);
                return SYS_OS;
            }
            
            break;
        }
    }
    
    cf_close(fpcfg);
    return 0;
}

Governor(sd_info_t *svcinfo)
{
    tskcmd_t *p_tskcmd = (tskcmd_t*)svcinfo->data;
    int rc, pid_index, taskid,pos, time_out=1;
    char buf[512 + 1]={0};
    char sSvrName[TUX_NAME_LEN+1]={0};
    FILE *fpcfg;

    p_tskcmd->err = 0;
    sighold(SIGTERM);
    sighold(SIGCLD);
    switch (p_tskcmd->cmd) 
    {
        case TSK_CMD_START:
        {
            SysLog(SYS_TRACE_NORMAL,"start task: id %d", p_tskcmd->id);
            taskid = find_server_id(p_tskcmd->id);
            if (taskid >= 0) 
            {
	    		sdGetSvrType(taskid) = SVR_PERM;
                /* get pid entry */
                pid_index = server_get_pidentry();
                if (pid_index < 0) 
                {
                    SysLog(SYS_TRACE_ERROR,"system task process number over limit");
                    p_tskcmd->err = SYS_LIMIT;
                    break;
                }
        
                SysLog(SYS_TRACE_NORMAL,"task id %d server  %s pid_tot  entry %d", sdGetSvrId(taskid), sdGetSvrName(taskid), sdGetSvrNum(taskid));
                
                sdGetSvrPidId(pid_index)    = sdGetSvrId(taskid);            /* task id */
                sdGetSvrPidPos(pid_index)   = taskid;                        /* id seq */
                sdGetSvrStat(pid_index)     = TUX_IDLE;   
                
                strncpy(sSvrName, sdGetSvrSvcName(taskid), TUX_NAME_LEN);
                time_out = tux_get_timeout(sSvrName);
                sdGetSvrTimeOut(taskid) = time_out;                     
                rc = server_start_pid(taskid, pid_index);
                if (rc) 
                {
                    sdSvrPidClear(pid_index);
                    SysLog(SYS_TRACE_ERROR,"task_tart_pid error %d", rc);
                    p_tskcmd->err = SYS_OS;
                    break;
                }
                
                sdGetSvrNum(taskid)++;
            } 
            else 
            {
                taskid = server_get_identry();
                if (taskid < 0) 
                {
                    SysLog(SYS_TRACE_ERROR,"system task number over limit");
                    p_tskcmd->err = SYS_LIMIT;
                    break;
                }

                p_tskcmd->err = run_one_task(taskid, p_tskcmd->id);
            }
            
            break;
        }
        case TSK_CMD_EXIT:
        {
            SysLog(SYS_TRACE_NORMAL,"exit task: id %d", p_tskcmd->id);
	    	taskid = find_server_id(p_tskcmd->id);
	    	if (taskid >= 0) 
	    	{
	    		long msg_type;
	    		sdGetSvrType(taskid) = SVR_TEMP;
	    		sdGetSvrStat(taskid) = TUX_DEAD;
	    		msg_type = sdGetSvrMsgType(taskid);
	    		server_kill(taskid, 1);
	    		
	    		clean_task_msg(msg_type);
	        } 
	        else 
	        {
	    	    SysLog(SYS_TRACE_ERROR,"task id %d no found", p_tskcmd->id);
	    	    p_tskcmd->err = SYS_NOENT;
	    	}
	    	
            break;
        }
        
        case TSK_CMD_FRESH:
        {
            SysLog(SYS_TRACE_NORMAL,"start task: id %d", p_tskcmd->id);
            taskid = find_server_id(p_tskcmd->id);
            if (taskid >= 0) 
            {
	    		sdGetSvrType(taskid) = SVR_TEMP;
	    		sdGetSvrStat(taskid) = TUX_DEAD;
	    		server_kill(taskid, 1);
	    		
                rc = run_task_id(taskid, sdGetSvrName(taskid));
                p_tskcmd->err = rc;

                break;
            } 
            else 
            {
                taskid = server_get_identry();
                if (taskid < 0) 
                {
                    SysLog(SYS_TRACE_ERROR,"system task number over limit");
                    p_tskcmd->err = SYS_LIMIT;
                    break;
                }
        
                p_tskcmd->err = run_one_task(taskid, p_tskcmd->id);
            }
            break;
        }
    }

    sigrelse(SIGCLD);
    sigrelse(SIGTERM);
    tux_return(TUX_SUCCESS, 0, svcinfo->data, svcinfo->len, 0);
}

void exit_handler(int sig)
{
    SysLog(SYS_TRACE_NORMAL,"receive SIGTERM signal, exit!");
    unload_server();
    sleep(1);
    exit(-1);
}

int main(int argc, char **argv)
{
    int rc;
    int svrid, svc_pos;
    char sLogFileName[128 + 1]={0};

    sprintf(sLogFileName, "%s.log", basename(argv[0]));
    strcpy(gsLogFile, sLogFileName);

    SysLog(SYS_TRACE_NORMAL,"server %s start up..............", basename(argv[0]));

    daemonize();

    sigset(SIGTERM, exit_handler);
    sighold(SIGCLD);
    sigset(SIGCLD, server_deatch);
    sighold(SIGUSR1);

    rc = sys_start();
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"sys_start error: %d", tux_errno);
        return -1;
    }

    rc = ipc_connect();
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"sys_init error: %d", tux_errno);
        return -1;
    }
    
    svrid = 0;
    svc_pos = 0;

    /* load server cfg */
    rc = server_load_cfg();
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"server_load_cfg error %d", rc);
        return rc;
    }

    rc = tux_advertise(SD_SERVER_MGR_NAME, Governor);
    if (rc) 
    {
        SysLog(SYS_TRACE_ERROR,"tux_advertise %s error %d", tux_errno);
        unload_server();
        sleep(1);
        return rc;
    }
    
    /* load all server */
    sighold(SIGTERM);
    sighold(SIGCLD);
    
    load_server();
    
    sigrelse(SIGCLD);
    sigrelse(SIGTERM);

    while (1) {
        rc = tux_service(svrid, svc_pos);
        if (rc != 0) {
            SysLog(SYS_TRACE_ERROR,"mb_service error %d", tux_errno);
            if (tux_errno != SYS_NOENT)
                break;
        }
    }

    unload_server();
    sleep(1);
    exit(-1);
}

/********************************************  文件结束 ******************************************/
