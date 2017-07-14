#ifndef  _TRACE_H
#define  _TRACE_H

#define  SYS_TRACE_OFF               0, __FILE__,  __LINE__    /* 不打印信息 */
#define  SYS_TRACE_ERROR             1, __FILE__,  __LINE__    /* 错误信息 */
#define  SYS_TRACE_KEY               2, __FILE__,  __LINE__    /* 关系信息 */
#define  SYS_TRACE_NORMAL            3, __FILE__,  __LINE__    /* 正常信息 */
#define  SYS_TRACE_DEBUG             4, __FILE__,  __LINE__    /* DEBUG信息 */

#define TRACE_LEN_MAX    64

char   gsLogFile[40+1];
char   gsMsgFile[40+1];

/*****************************************************
*函数名称: SysLog
*函数功能: 记录日志函数
*输入参数: sLogName -- 记录日志的文件
*输出参数：无
*函数返回: 无含义
*****************************************************/
extern int SysLog(char * vfmt , ...);

#endif
