/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：字符串处理公共函数
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo 建立.
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <iconv.h>
#include <errno.h>
#include "trace.h"

/*
 *  去除str结尾处的空字符
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
 *  去除str开头处的空字符
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
函数名称∶ AllTrim
函数功能∶ 删除字符串前后空格
输入参数∶ str -- 字符串
输出参数∶ str -- 处理后字符串
返 回 值∶ str  -- 处理后字符串
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
函数名称∶ IsDigitBuf
函数功能∶ 判断字符串是否都是数字
输入参数∶ sBuf -- 字符串
           nLen -- 校验长度
输出参数∶ 无
返 回 值∶ 0 -- 非
           1 -- 是
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



/******************************************************* 文件结束 **********************************/

