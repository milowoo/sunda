#ifndef _FUNSOCK_H_
#define _FUNSOCK_H_


/*****************************************************
*��������: sock_set_block
*��������: ��socket �趨������ģʽ
*�������: sockid     -- socket id
���������� on  -- �趨ģʽ 0 �Ƕ��� 1 ����
*�������  ��
*��������: 0  -- �ɹ�
           -1 -- ʧ��
*****************************************************/
extern int sock_set_block(int sockid, int on);

/*****************************************************
*��������: CreateSocket
*��������: ����socket,Listen�˿�
*�������: nPort     -- �˿ں�
���������� nRetryNum -- ʧ��ʱ�ظ��Ĵ���
           nWaitTime -- ʧ��ʱ�ظ��ȴ�ʱ��
           nBlock    -- ������־ 1 -- �Ƕ��� - ����
*�������  nSocket   -- socket������
*��������: 0  -- �ɹ�
           -1 -- ʧ��
*****************************************************/
extern int CreateSocket(unsigned int nPort, int nRetryNum, int nWaitTime, int nBlock, int *nSocket);




/*****************************************************
*��������: AcceptSocket
*��������: ����Զ�˿ͻ��˵�����
*�������: nSocket    -- socket������
           nWaitTime  -- ϵͳ����ȴ�ʱ��
           nNewSocket -- ��socket������
*�������  client���� -- �ͻ�����Ϣ
*��������: 0  -- �ɹ�
*****************************************************/
extern int AcceptSocket(int nSocket, struct sockaddr_in *client, int nWaitTime, int *nNewSocket);



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
extern int OpenSocket(char *sIpAddr, int nPort, int nRetrys, int nTimeOut, int *nSocket);



/*****************************************************
*��������: ReadSocket
*��������: ��socketָ�����ֽ�
*�������: nSocket  -- socket������
���������� nLen���� -- �ƶ���socket����
*�������  sBuf��   -- ��socket����
*��������: 0  -- �ɹ�
           -1 -- ʧ��
*****************************************************/
extern int ReadSocket(int nSocket, char *sBuf, int nLen);



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
extern int  RcvTxnMsg(int iSocketId, int iHeadLen, char *sSrcBuf, int * iBufLen);


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
extern int WriteSocket(int nSocket, char *sBuf, int nLen);


/*****************************************************
*��������: CloseSocket
*��������: �ر�socket��
*�������: nSocket  -- Ҫ�رյ�Socket��
*�������  ��
*��������: 0  -- �ɹ�
*****************************************************/
extern int CloseSocket(int nSocket);


#endif

