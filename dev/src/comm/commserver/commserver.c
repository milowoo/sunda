/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能: 通讯服务-- epoll 异步非堵塞短链接
 *
 *  Edit History:
 *
 *    2012/03/21 - MiloWoo 建立.
 *************************************************************************/
#include "sunda.h"
#include <pthread.h>

#define SOCKET_MSG_HEAD_LEN  4                 // msg head len
#define MAX_FD_NUM           1000              // child pid max epoll fd num  
#define MAX_EVENTS           8000              // epoll events num
#define MAX_TREAD_NUM        20

#define FD_STAT_REG       0
#define FD_STAT_POLL      1
#define FD_STAT_READ      2
#define FD_STAT_WRITE     3


typedef struct 
{                     
   int      fd;
   unsigned int  ssn;
   unsigned int  utime;  
   char     status;         // 0:register 1:POLL_IN 2:HAVE READ 3:HAVE WRITE
} connect_handle_t;

typedef struct
{
	int num;
	connect_handle_t connect[MAX_FD_NUM];
}client_handle_t;

static  client_handle_t client_ctrl[MAX_TREAD_NUM];

static  int   num;                   // 子进程个数
static  int   epfd;
static  int   idletime;              // 通讯占用时间--秒单位
static  int   port;                  // listen 通讯端口
static  int   listen_socket;         // listen sock_id 
static  int   giSvrId;                              
static  int   giTimeOut;
static  int   gssn;
static  int   gthread_num;

unsigned int s_thread_para[MAX_FD_NUM][8];
pthread_mutex_t  s_mutex[MAX_FD_NUM];
pthread_cond_t   s_read_cond[MAX_FD_NUM];
pthread_mutex_t  write_mutex;


#define CommHandClear(i)  do {memset(&client_ctrl[(i)], 0, sizeof(client_handle_t)); client_ctrl[(i)].num = 0; } while (0)
#define GetCommClientNum(i)    (client_ctrl[(i)].num) 
#define InitConnect(i,j)    do {client_ctrl[(i)].connect[(j)].fd =-1; client_ctrl[(i)].connect[(j)].status =-1; } while (0)    
#define GetCommClientFD(i,j)   (client_ctrl[(i)].connect[(j)].fd)
#define GetCommClientSsn(i,j)  (client_ctrl[(i)].connect[(j)].ssn)
#define GetCommClientTime(i,j)  (client_ctrl[(i)].connect[(j)].utime) 
#define GetCommClientStat(i,j)  (client_ctrl[(i)].connect[(j)].status) 
#define CommClearFd(i,j) \
             do {memcpy((char *)&client_ctrl[(i)].connect[(j)], (char *)&client_ctrl[(i)].connect[(j)+1], sizeof(connect_handle_t) *(MAX_FD_NUM - (i) -1)); \
             	client_ctrl[(i)].connect[MAX_FD_NUM-1].fd = -1; client_ctrl[(i)].connect[MAX_FD_NUM-1].status = -1;} while (0)

static int create_worker_process(int i);

static void time_out(int i)
{
	SysLog(SYS_TRACE_ERROR, "over time!");
	giTimeOut = 1;
}


static void main_exit(int sig)
{
    int i, j;
    struct epoll_event ev;
    
    sleep(2);
	
	ev.data.fd = listen_socket;
    epoll_ctl(epfd, EPOLL_CTL_DEL, listen_socket, &ev);

    SysLog(SYS_TRACE_ERROR,"main_exit ");

    for (i = 0; i < num; i++) 
    {
    	for (j = 0; j < MAX_FD_NUM; j++)
    	{
    		if (GetCommClientFD(i, j) == -1) 
    		    break;
    		    
    		CloseSocket(GetCommClientFD(i, j));
    	}
    	
    	
        
    }

    sleep(1);

    shutdown(listen_socket, 2);
    close(listen_socket);

    exit(0);
}


static unsigned int get_curr_time()
{
	struct timeval now;
	
	gettimeofday(&now, NULL);
    return now.tv_sec;
}


static int get_connect_pos(int idx, int fd)
{
	int i;
	
	for (i = 0; i < MAX_FD_NUM; i++) 
    {
    	if (fd != -1)
    	{
    		if (GetCommClientFD(idx, i) == fd)
    		    return i;
    	}
    	else
    	{
    		if (GetCommClientFD(idx, i) == -1)
    		    return i;
    	} 
    }
    
    if (i == MAX_FD_NUM)
    {
    	return -1;
    }
}

static int get_conn_inf(int sock_id, unsigned int ssn)
{
	int i, idx;
	idx = sock_id % num;
	for (i = 0; i < MAX_FD_NUM; i++) 
    {
    	if (GetCommClientFD(idx, i) != -1)
    	{
    		if (GetCommClientSsn(idx, i) == ssn)
    		    return i;
    	}
    	else
    	{
    		return -1;
    	} 
    }
}
 

static int new_accept(int epfd)
{
	int iRet, pos, sockfd, idx;
	struct epoll_event ev;
	socklen_t clilen;
	struct sockaddr_in clientaddr;

	bzero(&clientaddr, sizeof(clientaddr));
	iRet = AcceptSocket(listen_socket, &clientaddr, 2, &sockfd);
	if( iRet < 0 )
	{
		SysLog(SYS_TRACE_ERROR, "AcceptSocket errno %d = %s", errno, strerror(errno));
		return -1;
	}
	
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
	{
		SysLog(SYS_TRACE_ERROR, "set non_block ");
		CloseSocket(sockfd);
		return -1;
	}
    
    idx = sockfd % num;
//    pthread_mutex_lock(s_mutex + idx);
	pos = get_connect_pos(idx, -1);
	if (pos < 0)
	{
	//	pthread_mutex_unlock(s_mutex + idx);
		SysLog(SYS_TRACE_ERROR, "get_connect_pos have no idle connect pos ");
		CloseSocket(sockfd);
		return -1;
	}
	
	ev.data.fd = sockfd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	
	GetCommClientFD(idx, pos) = sockfd;
	GetCommClientSsn(idx, pos) = gssn++;
	GetCommClientTime(idx, pos) = get_curr_time();
	GetCommClientStat(idx, pos) = FD_STAT_REG;
	// GetCommClientNum(idx) = GetCommClientNum(idx)+1;
	
//	pthread_mutex_unlock(s_mutex+idx);
	
	SysLog(SYS_TRACE_ERROR, "new accept fd%d  end", sockfd);

	return 0;
}

static int response_err_req(int socketid, sunda_req_head_t *req_head)
{
	int iRet;
	char iLen[SOCKET_MSG_HEAD_LEN+1];
	char sRspBuf[1024+1]={0};
	
	sprintf(iLen,"%0*d",SOCKET_MSG_HEAD_LEN, sizeof(sunda_req_head_t));
	
	memcpy(sRspBuf, iLen, SOCKET_MSG_HEAD_LEN);
	memcpy(sRspBuf+SOCKET_MSG_HEAD_LEN, (char *)req_head, sizeof(sunda_req_head_t));
	
	// alarm(1);
    iRet = WriteSocket(socketid, sRspBuf, sizeof(sunda_req_head_t)+SOCKET_MSG_HEAD_LEN);
    //alarm(0);
    if (iRet < 0)
    { 
    	SysLog(SYS_TRACE_ERROR , "WriteSocket error %d  error = %s.", errno, strerror(errno));
    	return -1;
    }
	
	return 0;
}

static int del_connect_fd(int epfd, int fd)
{
	int i, idx;
	struct epoll_event ev;
	
	idx = fd % num;
	
	ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    CloseSocket(fd);
	
	for (i = 0; i < MAX_FD_NUM; i++) 
    {
    	if (GetCommClientFD(idx,i) == -1)
    	{
    		break;
    	}
    	
        if (GetCommClientFD(idx, i) == fd)
        {
        	if (GetCommClientStat(idx,i) == FD_STAT_POLL)
        	    GetCommClientNum(i) = (GetCommClientNum(i) == 0 ) ? 0 : (GetCommClientNum(i) - 1);
            CommClearFd(idx, i) ;
            break;
        }
    }
    
    return 0;
}

void clean_time_out_connect(int epfs)
{
	int i, j, fd;
	unsigned int utime;
	struct epoll_event ev;
	unsigned int currtime;
	unsigned int diff_time = 0;
	
	currtime = get_curr_time();	
	for (i = 0; i < num; i++) 
    {
    	for (j = 0; j < MAX_FD_NUM; j++)
    	{
    		fd = GetCommClientFD(i,j);
    		if (fd == -1)
    		{
    			break;
    		}
            else 
            {
            	diff_time = currtime - GetCommClientTime(i, j);
            	if (diff_time >= idletime)
            	{
            		SysLog(SYS_TRACE_ERROR , "fd = %d utime =[%d]", fd, diff_time);
            		ev.data.fd = fd;
                    epoll_ctl(epfs, EPOLL_CTL_DEL, fd, &ev);
                    CloseSocket(fd);
            		CommClearFd(i,j);
            		j = j - 1;
            	}
            	else
            	{
            		break;
            	}
            }  
    	}
    	
    }
    
    return ;
}

void * write_task(void *args)
{
	int   iRet, pos, msg_type, idx, iRspLen = 0;
    long  iBufLen = 0;
    char  sSrcBuf[10240+1]={0};
    char  sRspBuf[10240+1]={0};
    IPCMsgHead ipcMsgHead={0};
    sunda_req_head_t req_head={0};
    
    pthread_detach(pthread_self());
    
    while (1)
    {   
        msg_type = COMM_S_MSG_TYPE_ROOT;
        memset(sSrcBuf, 0, sizeof(sSrcBuf));
        pthread_mutex_lock(&write_mutex);
        iRet = tux_getreply(msg_type, sSrcBuf, &iBufLen, 0);
        pthread_mutex_unlock(&write_mutex);
        if (iRet != 0)
        {
        	if (tux_errno == SYS_SYSTEM)
        	{
        		SysLog(SYS_TRACE_ERROR , "tux_getreply system err tux_errno [%d]" , tux_errno);
        		return;
        	}
        	else if (tux_errno == ENOMSG)
        	{
        		continue;
        	}
        }

        memset(sRspBuf, 0, sizeof(sRspBuf));
        memset(&ipcMsgHead, 0, sizeof(ipcMsgHead));
        memset(&req_head, 0, sizeof(req_head));
        
        memcpy((char *)&ipcMsgHead, sSrcBuf, sizeof(ipcMsgHead));
        memcpy(req_head.name, ipcMsgHead.name, TUX_NAME_LEN);
        req_head.ssn = ipcMsgHead.ssn;
        req_head.err_no = 0;
        
        iRspLen = iBufLen-sizeof(gipcMsgHead)+sizeof(req_head);
        sprintf(sRspBuf, "%0*d", SOCKET_MSG_HEAD_LEN, iRspLen);
        memcpy((char *)sRspBuf+SOCKET_MSG_HEAD_LEN, (char *)&req_head, sizeof(req_head));
        memcpy(sRspBuf+(sizeof(req_head)+SOCKET_MSG_HEAD_LEN), sSrcBuf+sizeof(gipcMsgHead), iRspLen);
        iRspLen = iRspLen + SOCKET_MSG_HEAD_LEN;
        
        idx = ipcMsgHead.fd % num;
        pthread_mutex_lock(s_mutex + idx);
        pos = get_conn_inf(ipcMsgHead.fd, ipcMsgHead.conn_ssn);
        if (pos < 0)
        {
        	SysLog(SYS_TRACE_ERROR, "get_conn_inf err ssn [%d] ", req_head.ssn);
        	pthread_mutex_unlock(s_mutex + idx);
            continue;
        }
        
        GetCommClientStat(idx, pos) = FD_STAT_WRITE;

        pthread_mutex_unlock(s_mutex + idx);
        
        iRet = WriteSocket(ipcMsgHead.fd, sRspBuf, iRspLen);
        if (iRet < 0)
        {
        	SysLog(SYS_TRACE_ERROR , "WriteSocket error %d  error = %s.", errno, strerror(errno));
        }
        
    }

    return 0;
}

int getconnetcount(int idx)
{
	int i;
	int number = 0;
	
	for (i = 0; i < MAX_FD_NUM; i++)
	{
		if (GetCommClientFD(idx, i) != -1)
			number++;
		else
			break;
	}
	
	return number;
}

static void get_wait_time(int *timeout)
{
	int i = 0, number = 0;
	unsigned int currtime;
	int diff_time = 0;
	int tmp_time = 0;
    
    currtime = get_curr_time();	 	  
    for (i = 0; i < num; i++) 
    {
    	if (GetCommClientFD(i, 0) != -1)
    	{
    		number++;
    		if (currtime >= (GetCommClientTime(i, 0) + idletime))
    		    tmp_time = currtime - (GetCommClientTime(i, 0) + idletime);
    		else 
    			tmp_time = 0;
    			
    		if (tmp_time >= diff_time)
    			diff_time = tmp_time;
    	}
    }

    if (number == 0)
    	*timeout = -1;
    else
       *timeout = diff_time * 1000 + 50;
	return;
}

static int get_read_fd(int idx)
{
	int i;
	for (i = 0; i < MAX_FD_NUM; i++) 
    {
    	if (GetCommClientFD(idx, i) < 0)
    		break;
    		
        if (GetCommClientStat(idx, i) == FD_STAT_POLL)
        {
            return i;
        }    
    }
    
    return -1;
}

void * read_task(unsigned int thread_para[])
{
   int socketid, iRet, pos, iBufLen, idx;
   long lReqLen = 0;
   char sSrcBuf[8096+1]={0};
   char sReqBuf[10240+1]={0};
   char sSvcName[TUX_NAME_LEN+1]={0};
   IPCMsgHead ipcMsgHead={0};
   sunda_req_head_t req_head={0};
   connect_handle_t  tClient;
   
   pthread_detach(pthread_self());
   idx = thread_para[7];
   
   while(1)
   {
   	    memset(&tClient, 0, sizeof(tClient));
   	    
   	    pthread_mutex_lock(s_mutex + idx);
   	    while (GetCommClientNum(idx) == 0)
   	    {
   	    	SysLog(SYS_TRACE_ERROR, "read_task_%d waite to read", idx);
   	    	pthread_cond_wait(s_read_cond+idx, s_mutex + idx);
   	    }
   	    
        SysLog(SYS_TRACE_ERROR, "read_task read begin");
        
        if ((pos = get_read_fd(idx)) == -1)
        {
        	client_ctrl[idx].num = 0;
        	pthread_mutex_unlock(s_mutex + idx);
        	continue;
        }
       
        socketid = GetCommClientFD(idx, pos);
        GetCommClientStat(idx,pos) = FD_STAT_READ;
        memcpy((char *)&tClient, (char *)&client_ctrl[idx].connect[pos], sizeof(connect_handle_t));
        GetCommClientNum(idx) = GetCommClientNum(idx) - 1;
        
        pthread_mutex_unlock(s_mutex + idx);
        
        memset(sSrcBuf, 0, sizeof(sSrcBuf));
        iBufLen= 0;
        iRet = RcvTxnMsg(socketid, SOCKET_MSG_HEAD_LEN, sSrcBuf, &iBufLen);
        if (iRet !=  0)
        {
            SysLog(SYS_TRACE_ERROR, "RcvTxnMsg err [%d] ", iRet);
            pthread_mutex_lock(s_mutex + idx);
            del_connect_fd(epfd, socketid);
            pthread_mutex_unlock(s_mutex + idx);
            continue;
        }
        
        memset(&req_head, 0, sizeof(req_head));
        memcpy((char *)&req_head, sSrcBuf+SOCKET_MSG_HEAD_LEN, sizeof(req_head));
        
        memset(sSvcName, 0, sizeof(sSvcName));
        memcpy(sSvcName, req_head.name, TUX_NAME_LEN);
        AllTrim(sSvcName);
        
        memset(&ipcMsgHead, 0, sizeof(ipcMsgHead));
        strcpy(ipcMsgHead.name, sSvcName);
        ipcMsgHead.req_msg_type = COMM_S_MSG_TYPE_ROOT;
        ipcMsgHead.fd  = socketid; 
        ipcMsgHead.ssn = req_head.ssn;
        ipcMsgHead.conn_ssn = tClient.ssn;
        ipcMsgHead.utime  = tClient.utime;
        ipcMsgHead.reply_flag = REPLY_FLAG_ASYN;
        ipcMsgHead.len = iBufLen -  SOCKET_MSG_HEAD_LEN - sizeof(req_head);
        memcpy(sReqBuf, (char *)&ipcMsgHead, sizeof(ipcMsgHead));
        memcpy(sReqBuf+sizeof(ipcMsgHead),  sSrcBuf+(SOCKET_MSG_HEAD_LEN + sizeof(req_head)), iBufLen);
        lReqLen = iBufLen+sizeof(ipcMsgHead);
        
        iRet = tux_acall(sSvcName, sReqBuf, lReqLen, 0);
        if (iRet != 0)
        {
        	req_head.err_no = tux_errno;
        	response_err_req(socketid, &req_head);
        	SysLog(SYS_TRACE_ERROR , "tux_acall err %d !", tux_errno);
        	continue;
        }
   }
}

static int comm_init(int argc , char **argv)
{
	int iRet, i, ch, j;
	
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
    
    giSvrId = atoi(argv[3]);

    memset(gsLogFile, 0, sizeof(gsLogFile));
    sprintf(gsLogFile, "short_comm_%d.log", giSvrId);
	
	while ((ch = getopt(argc - 2, argv + 2, "p:n:t:")) != -1) 
    {
        switch (ch) 
        {
        case 'p':
            port = atoi(optarg);
            SysLog(SYS_TRACE_NORMAL,"server listen port %d\n", port);
            break;
        case 'n':
            num = atoi(optarg);
            if (num <= 0) 
            {
                SysLog(SYS_TRACE_ERROR,"USAGE %s -p PORT -n NUMBER -t IDLETIME \n", argv[0]);
                return -1;
            }
            
            if (num >= MAX_TREAD_NUM)
            {
            	num = MAX_TREAD_NUM;
            }
            
            SysLog(SYS_TRACE_NORMAL,"server fork work process %d\n", num);
            break;
        case 't':
            idletime = atoi(optarg);
            SysLog(SYS_TRACE_NORMAL,"client max ldle time %d\n", idletime);
            break;
        }
    }
    
    gssn = 0;
    
    iRet = ipc_connect();
    if (iRet) 
    {
        SysLog(SYS_TRACE_ERROR,"ipc_connect error");
        return -1;
    }
    
    for (i = 0; i < num; i++) 
    {
    	CommHandClear(i);
    	for (j = 0; j < MAX_FD_NUM; j++)
        {
        	InitConnect(i,j);
        }
    }
   
    return 0;
}

static int init_thread_pool(void)
{
	int i, rc;
	pthread_t s_tid_r[MAX_TREAD_NUM];
	pthread_t s_tid_w[MAX_TREAD_NUM];
    for(i = 0; i < num; i++) 
    {
        s_thread_para[i][0] = 0;
        s_thread_para[i][7] = i; 
        pthread_cond_init(s_read_cond+i, NULL);
    }
    
    for(i = 0; i < num; i++) 
    {
        rc = pthread_create(s_tid_r + i, 0, (void *)read_task, (void *)(s_thread_para[i]));
        if (0 != rc) 
        {
            SysLog(SYS_TRACE_ERROR,"pthread_create  read_task err ");
            return(-1);
        }
    }
    
    for(i = 0; i < 1; i++)
    {
    	rc = pthread_create(s_tid_w+i, NULL, write_task, NULL);
    	if (0 != rc) 
        {
            SysLog(SYS_TRACE_ERROR,"pthread_create write_task err ");
            return(-1);
        }
    }
    
    pthread_mutex_init(&write_mutex, NULL);
    
    return(0);
}

int main(int argc, char **argv)
{
    int    iRet, i, timeout, nfds, pos, idx;
    struct epoll_event ev, events[MAX_EVENTS];
    pthread_t tid1,tid2;

    memset(gsLogFile, 0, sizeof(gsLogFile));
    strcpy(gsLogFile, "ULOG");
    
    iRet = comm_init(argc, argv);
    if (iRet != 0)
    {
    	SysLog(SYS_TRACE_ERROR,"comm_init err ");
        return -1;
    }
    
    SysLog(SYS_TRACE_ERROR,"comm_init success ");
    
    iRet = CreateSocket(port, 9999, 1, 1, &listen_socket);
    if (iRet != 0) 
    {
        SysLog(SYS_TRACE_ERROR,"bind socket port %d iRet =%d error %d %s\n", port, iRet,  errno, strerror(errno));
        exit(2);
    }
    
    epfd = epoll_create( MAX_EVENTS );
	ev.data.fd = listen_socket;
	ev.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_socket, &ev ) < 0)
	{
		SysLog(SYS_TRACE_ERROR, "worker epoll_ctl error %d  error = %s.", errno, strerror(errno));
		close(epfd);
		exit(1);
	}
    
    sigset(SIGTERM, main_exit);
    
    //初始化用于读线程池的线程
    iRet = init_thread_pool();
    if (iRet != 0)
    {
    	SysLog(SYS_TRACE_ERROR,"init_thread_pool err ");
    	close(epfd);
        return -1;
    }
    
    while (1) 
    {
        get_wait_time(&timeout);
        nfds = epoll_wait(epfd, events, 200, timeout);
		for( i=0; i< nfds; i++) 
		{
			idx = events[i].data.fd % num;
			if(events[i].data.fd == listen_socket)
			{
				SysLog(SYS_TRACE_ERROR,"new_accept begin ");
				iRet = new_accept(epfd);
				if (iRet != 0)
				{
					SysLog(SYS_TRACE_ERROR , "new_accept err");
        	        events[i].data.fd = -1;
				}
				
				continue;
			} 
#if 0
			else if (events[i].events & EPOLLHUP)
			{	
				SysLog(SYS_TRACE_ERROR,"fd exit ");
				pthread_mutex_lock(s_mutex+idx);
				del_connect_fd(epfd, events[i].data.fd);
				pthread_mutex_unlock(s_mutex+idx); 
				events[i].data.fd = -1;
				
        	    continue;
			}
#endif

			else if (events[i].events & EPOLLIN)
			{
				SysLog(SYS_TRACE_ERROR,"begin to read ");
				pthread_mutex_lock(s_mutex+idx);
				pos = get_connect_pos(idx, events[i].data.fd);
				if (pos < 0)
				{
					del_connect_fd(epfd, events[i].data.fd);
					SysLog(SYS_TRACE_ERROR , "get_connect_pos err %d !", tux_errno);
        	        events[i].data.fd = -1;
        	        pthread_mutex_unlock(s_mutex+idx);
        	        continue;
				}
				
				GetCommClientStat(idx, pos) = FD_STAT_POLL;
				GetCommClientNum(idx) = GetCommClientNum(idx) + 1;
				pthread_cond_broadcast(s_read_cond + idx);
				pthread_mutex_unlock(s_mutex+idx); 
			}			
			else
			{
				SysLog(SYS_TRACE_ERROR , "not found the err !");
				pthread_mutex_lock(s_mutex+idx);
				del_connect_fd(epfd, events[i].data.fd);
				pthread_mutex_unlock(s_mutex+idx); 
				events[i].data.fd = -1;
        	    continue;
			}	
		}
		
		clean_time_out_connect(epfd);
	}
    
    return 0;
}

/******************************************* 文件结束 **********************************************/

