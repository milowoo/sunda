/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  ���ܣ���־������������
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo ����.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "trace.h"

#define LOG_SIZE_UNIT 1000000


static int GetMsgFileName(int iLevel, char *sFileName);
static int GetTraceFileName(int iLevel, char *sFileName);

/*****************************************************
*��������: BatLog
*��������: ��¼��־����
*�������: sLogName -- ��¼��־���ļ�
*�����������
*��������: �޺���
*****************************************************/
int SysLog(char * vfmt , ...)
{
    va_list   args;
    int       nLevel;             /* ��־���� */
    int       nSysLevel;
    FILE      *fp;
    char      *fmt;               /* ��־���ݵĸ�ʽ���ַ��� */
    char      *file;
    int       line;
    char      sFileName[128 + 1];
    char      sCurrTime[14 + 1];
    char      sMessage[10240 + 1];

    nLevel = vfmt;

    va_start(args , vfmt);
    file = va_arg(args, char*);
    line = va_arg(args, int);
    fmt = va_arg(args, char*);
    vsprintf(sMessage, fmt, args);
    va_end(args);

    if (getenv("LOG_MODE") == NULL)
    {
        nSysLevel = 9;
    }
    else
    {
        nSysLevel = atoi(getenv("LOG_MODE"));
    }

    if (nLevel == 0)
    {
    	return 0;
    }

    if (nLevel > nSysLevel)
    {
        return 0;
    }

    memset(sFileName, 0, sizeof(sFileName));
    GetTraceFileName(nLevel, sFileName);

    fp = fopen(sFileName, "a+");
    if (fp != (FILE*)NULL)
    {
		GetCurrentTime(sCurrTime);
        fprintf(fp, "[%06d][%s][%15.15s][%05d] %s\n", getpid(), sCurrTime, file , line , sMessage);
        fclose(fp);
        return 0;
    }

    return 0;
}


/*****************************************************
*��������: GetTraceFileName
*��������: ��¼��־����
*�������: sLogName  -- ��¼��־���ļ�
           iLevel    -- ��¼��־����
*���������sFileName -- ��־�ļ���ȫ·��
*��������: �޺���
*****************************************************/
static int GetTraceFileName(int iLevel, char *sFileName)
{
    int iRet;
    char sDateTime[14 + 1]={0};
    char sNewFileName[128+1]={0};
    struct stat	statbuf={0};


    chdir(getenv("PWD"));

    /* ������־ */
    GetDate(sDateTime);
    sprintf(sFileName, "%s/log", (char *)getenv("HOME"));
    chdir(sFileName);

    /* �����ڴ�� */
    strcat(sFileName, "/");
    strcat(sFileName, sDateTime);
    mkdir(sFileName, S_IRWXU|S_IRWXG|S_IROTH);

    /* �ļ���+.log���� */
    strcat(sFileName, "/");
    strcat(sFileName, gsLogFile);

    /* �ж�ԭ��־�ļ��Ƿ����,������б��� */
    iRet = stat (sFileName, &statbuf);
    if (iRet == 0 && statbuf.st_size >= LOG_SIZE_UNIT * 5 )
	{
	    GetCurrentTime(sDateTime);
		sprintf (sNewFileName, "%s.%4.4s", sFileName, sDateTime+8);
		rename (sFileName, sNewFileName);
	}

    return 0;
}


/*****************************************************
*��������: GetMsgFileName
*��������: ��¼��־����
*�������: sLogName  -- ��¼��־���ļ�
           iLevel    -- ��¼��־����
*���������sFileName -- ��־�ļ���ȫ·��
*��������: �޺���
*****************************************************/
static int GetMsgFileName(int iLevel, char *sFileName)
{
    char    sDateTime[14 + 1];

    memset(sDateTime, 0x00, sizeof(sDateTime));
    chdir(getenv("PWD"));

    /* ����������������Ŀ¼ */
    GetDate(sDateTime);
    sprintf(sFileName, "%s", (char *)getenv("HOME"));
    chdir(sFileName);

    /* �����ڴ�� */
    strcat(sFileName, "/");
    strcat(sFileName, sDateTime);
    mkdir(sFileName, S_IRWXU|S_IRWXG|S_IROTH);

    /* �ļ���+.log���� */
    strcat(sFileName, "/");
    strcat(sFileName, gsMsgFile);

    return 0;
}

/*****************************************************
*��������: HtTrace
*��������: ��¼Trace��־����
*�������: sLogName   -- ��¼��־���ļ�
           nLevel     -- ��־����
           sFileName  -- Դ�����ļ���
           nLine      -- ��¼��־����
           sBuf       -- ��¼��־����
           nLen       -- ��ӡtrace����
*�����������
*��������: �޺���
*****************************************************/
int SysTrace(int nLevel,char *sFileName, int nLine , char *sBuf, int nLen )
{
    char      sTraceFileName[128 + 1];
    int       i, j = 0;
    char      s[100], Temp[5];
    FILE      *fp = NULL;
    char      sCurrTime[6 + 1];
    int       nSysLevel;

    memset(sTraceFileName, 0, sizeof(sTraceFileName));

    if (getenv("LOG_MODE") == NULL)
    {
        nSysLevel = 0;
    }
    else
    {
        nSysLevel = atoi(getenv("LOG_MODE"));
    }

    if (nLevel == 0)
    {
    	return 0;
    }

    if (nLevel > nSysLevel)
    {
        return 0;
    }

    GetMsgFileName(nSysLevel, sTraceFileName);

    if ((fp = fopen(sTraceFileName, "a+")) == NULL)
    {
        return 0;
    }

	GetCurrentTime(sCurrTime);
    fprintf(fp, "(%06d)[%s][%s][%d]\n", getpid(), sCurrTime, sFileName, nLine);

    memset(s, 0, sizeof(s));
    memset(s, '-', 80);
    fprintf(fp, "%s\n", s);

    for (i=0; i<nLen; i++)
    {
        if (j == 0)
        {
            memset( s, ' ', sizeof(s));
            sprintf(Temp,   "%04d:",i );
            memcpy( s, Temp, 5);
            sprintf(Temp,   ":%04d",i+15 );
            memcpy( &s[72], Temp, 5);
        }

        sprintf( Temp, "%02X ", (unsigned char)sBuf[i]);
        memcpy( &s[j*3+5+(j>7)], Temp, 3);

        if ( isprint( sBuf[i]))
        {
            s[j+55+(j>7)]=sBuf[i];
        }
        else
        {
            s[j+55+(j>7)]='.';
        }

        j++;

        if (j == 16)
        {
            s[77]=0;
            fprintf(fp, "%s\n", s);
            j=0;
        }
    }

    if (j)
    {
        s[77]=0;
        fprintf(fp, "%s\n", s);
    }

    memset(s, 0, sizeof(s));
    memset(s, '-', 80);
    fprintf(fp, "%s\n", s);
    fflush(fp);

    fclose(fp);

    return 0;
}



/************************************* �ļ����� *******************************************/
