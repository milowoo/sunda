/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  ���ܣ��ַ�������������
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo ����.
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <iconv.h>
#include <errno.h>
#include "trace.h"

/*
 *  ȥ��str��β���Ŀ��ַ�
 */
char *RightTrim(char *sBuf)
{
    char    *s = sBuf;

    while (*s)
      ++s;

    --s;
    while (s >= sBuf)
        if ( (*s==' ') || (*s=='\t') || (*s=='\r') || (*s=='\n') )
           --s;
        else
            break;

    *(s + 1) = 0;
    return sBuf;
}


/*
 *  ȥ��str��ͷ���Ŀ��ַ�
 */
char * LeftTrim( char *str )
{
    char    *s = str;

    while ( *s )
        if ( (*s==' ') || (*s=='\t') || (*s=='\r') || (*s=='\n') )
          ++s;
        else
            break;

    if ( s > str )
        strcpy ( (char *)str, (char *)s );

    return str;
}

/*****************************************************************************
�������ơ� AllTrim
�������ܡ� ɾ���ַ���ǰ��ո�
��������� str -- �ַ���
��������� str -- ������ַ���
�� �� ֵ�� str  -- ������ַ���
*****************************************************************************/
char * AllTrim(char *str)
{
    int i, j, k, h;

    for (i = 0; str[i] == ' ' && str[i] != '\0'; i++);
    for (j = strlen(str) - 1; str[j] == ' ' && j >= 0 ; j--);

    for (k = 0; k <= j - i; k++)
        str[k] = str[k + i];
    str[k] = '\0';

    while(str[k] != 0){
        str[k] = 0;
        k++;
    }

    return str;
}

/*****************************************************************************
�������ơ� IsDigitBuf
�������ܡ� �ж��ַ����Ƿ�������
��������� sBuf -- �ַ���
           nLen -- У�鳤��
��������� ��
�� �� ֵ�� 0 -- ��
           1 -- ��
*****************************************************************************/
int IsDigitBuf(char *sBuf, int nLen)
{
    int    i;

    if (nLen == 0)
        return 1;

    for (i = 0; i < nLen; i++)
        if (!isdigit(sBuf[i]))
            return 0;

    return 1;
}



/******************************************************* �ļ����� **********************************/

