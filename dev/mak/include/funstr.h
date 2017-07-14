#ifndef _FUNSTR_H_
#define _FUNSTR_H_

/*
 *  去除str结尾处的空字符
 */
extern char *RightTrim(char *sBuf);



/*
 *  去除str开头处的空字符
 */
extern char * LeftTrim( char *str );

/*****************************************************************************
函数名称∶ AllTrim
函数功能∶ 删除字符串前后空格
输入参数∶ str -- 字符串
输出参数∶ str -- 处理后字符串
返 回 值∶ str  -- 处理后字符串
*****************************************************************************/
extern char * AllTrim(char *str);

#endif

