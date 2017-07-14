/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能: 系统管理命令操作函数
 *
 *  Edit History:
 *
 *    2012/03/16 - MiloWoo 建立.
 *************************************************************************/
#include "sunda.h"

char **make_argv(char *buf, int usefirst)
{
    int   i;
    char  *p;
    char  **argv;

    if (usefirst <= 0) return NULL;

    argv = NULL;
    /* the max parameter number is 10 */
    argv = (char **)malloc(sizeof(char *) * 30);
    RightTrim(buf);

    i = 0;
    p = strtok(buf, " ");
    while (p) {
        argv[i] = p;
        i++;
        p = strtok(NULL, " ");
    }
    argv[i] = NULL;
    return &argv[usefirst - 1];
}

int server_load(const char * tskname , const char * arg)
{
    pid_t pid;
    char  **arg_v = NULL;
    char  buf[128 + 1]={0};

    /* start task */
    pid = fork();
    if (pid == -1) 
    {
        SysLog(SYS_TRACE_ERROR,"fork error: %d, %s.", errno, strerror(errno));
        return -1;
    } 
    else if (pid > 0) 
    {
        SysLog(SYS_TRACE_ERROR,"**********fork to start task: %s %s success, pid: %d.", tskname, arg, pid);
        return pid;
    } 
    else 
    {
        strcpy(buf, tskname);
        strcat(buf, " ");
        sprintf(buf + strlen(buf), "%ld", getpid());
        strcat(buf, " ");
        strcat(buf, arg);
        SysLog(SYS_TRACE_DEBUG,"execvp %s %s", tskname, buf);
        arg_v = make_argv(buf, 1);
        if (execvp(tskname, arg_v) == -1) 
        {
            free(arg_v);
            SysLog(SYS_TRACE_ERROR,"**********start task fail at execvp: %s %s error: %d, %s.", tskname, arg, errno, strerror(errno));
        }
        free(arg_v);
        exit(1);
    }
}

static int packReqHead(IPCMsgHead *ipcMsgHead, long lMsgType)
{
	strcpy(ipcMsgHead->name, SD_MANAGER_SVC_NAME);
	ipcMsgHead->ssn  = time(NULL);
    ipcMsgHead->req_msg_type  = lMsgType;
    ipcMsgHead->fd  = 0;
    ipcMsgHead->utime  = time(NULL);
    ipcMsgHead->reply_flag = REPLY_FLAG_SYN;
    ipcMsgHead->len = sizeof(tskcmd_t);
    return 0;
}

int svr_cmd_deal(int id , int cmd)
{
	int rc;
	long  msg_typ, rsq_len, req_len;
    tskcmd_t tTaskCmd={0};
    char sReqBuf[1024+1]={0};
    char sRspBuf[1024+1]={0};
    
    memset(&gipcMsgHead, 0, sizeof(gipcMsgHead));

    packReqHead(&gipcMsgHead, cmd+MNG_MSG_TYPE_ROOT);
    tTaskCmd.cmd = cmd;
    tTaskCmd.id  = id;
    memcpy(sReqBuf, (char *)&gipcMsgHead, sizeof(gipcMsgHead));
    memcpy((char *)sReqBuf+sizeof(gipcMsgHead), (char *)&tTaskCmd, sizeof(tTaskCmd));
    
    req_len = sizeof(tskcmd_t)+sizeof(gipcMsgHead);
    rc = tux_call(SD_MANAGER_SVC_NAME, (char*)sReqBuf, req_len, sRspBuf, &rsq_len, 0);
    if (rc != 0) 
    {
        printf ("tux_call %s error: %d \n ", SD_MANAGER_SVC_NAME, rc);
        return rc;
    }
    
    memcpy((char *)&tTaskCmd, (char *)sRspBuf+sizeof(gipcMsgHead), sizeof(tTaskCmd));

    return tTaskCmd.err;
}

