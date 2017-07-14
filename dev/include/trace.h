#ifndef  _TRACE_H
#define  _TRACE_H

#define  SYS_TRACE_OFF               0, __FILE__,  __LINE__    /* ����ӡ��Ϣ */
#define  SYS_TRACE_ERROR             1, __FILE__,  __LINE__    /* ������Ϣ */
#define  SYS_TRACE_KEY               2, __FILE__,  __LINE__    /* ��ϵ��Ϣ */
#define  SYS_TRACE_NORMAL            3, __FILE__,  __LINE__    /* ������Ϣ */
#define  SYS_TRACE_DEBUG             4, __FILE__,  __LINE__    /* DEBUG��Ϣ */

#define TRACE_LEN_MAX    64

char   gsLogFile[40+1];
char   gsMsgFile[40+1];

/*****************************************************
*��������: SysLog
*��������: ��¼��־����
*�������: sLogName -- ��¼��־���ļ�
*�����������
*��������: �޺���
*****************************************************/
//extern int SysLog(char * vfmt , ...);

#endif
