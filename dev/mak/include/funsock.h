#ifndef _FUNSOCK_H_
#define _FUNSOCK_H_


/*****************************************************
*函数名称: sock_set_block
*函数功能: 给socket 设定非阻塞模式
*输入参数: sockid     -- socket id
　　　　　 on  -- 设定模式 0 非堵塞 1 堵塞
*输出参数  无
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
extern int sock_set_block(int sockid, int on);

/*****************************************************
*函数名称: CreateSocket
*函数功能: 建立socket,Listen端口
*输入参数: nPort     -- 端口号
　　　　　 nRetryNum -- 失败时重复的次数
           nWaitTime -- 失败时重复等待时间
           nBlock    -- 堵塞标志 1 -- 非堵塞 - 堵塞
*输出参数  nSocket   -- socket描述符
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
extern int CreateSocket(unsigned int nPort, int nRetryNum, int nWaitTime, int nBlock, int *nSocket);




/*****************************************************
*函数名称: AcceptSocket
*函数功能: 接收远端客户端的连接
*输入参数: nSocket    -- socket描述符
           nWaitTime  -- 系统出错等待时间
           nNewSocket -- 新socket描述符
*输出参数  client　　 -- 客户端信息
*函数返回: 0  -- 成功
*****************************************************/
extern int AcceptSocket(int nSocket, struct sockaddr_in *client, int nWaitTime, int *nNewSocket);



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
extern int OpenSocket(char *sIpAddr, int nPort, int nRetrys, int nTimeOut, int *nSocket);



/*****************************************************
*函数名称: ReadSocket
*函数功能: 读socket指定个字节
*输入参数: nSocket  -- socket描述符
　　　　　 nLen　　 -- 制定读socket长度
*输出参数  sBuf　   -- 读socket内容
*函数返回: 0  -- 成功
           -1 -- 失败
*****************************************************/
extern int ReadSocket(int nSocket, char *sBuf, int nLen);



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
extern int  RcvTxnMsg(int iSocketId, int iHeadLen, char *sSrcBuf, int * iBufLen);


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
extern int WriteSocket(int nSocket, char *sBuf, int nLen);


/*****************************************************
*函数名称: CloseSocket
*函数功能: 关闭socket号
*输入参数: nSocket  -- 要关闭的Socket号
*输出参数  无
*函数返回: 0  -- 成功
*****************************************************/
extern int CloseSocket(int nSocket);


#endif

