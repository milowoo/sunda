#ifndef _FUNTIME_H_
#define _FUNTIME_H_

/***************************************************
*函数名称: GetDate
*函数功能: 获取当天时间(YYYYMMDD)
*输入参数: sDate -- 日期 
*输出参数: sDate -- 日期 yyyymmdd
*返 回 :   当天时间
 ***************************************************/
char *GetDate( char * buff);

/***************************************************
*函数名称: GetCurrentTime
*函数功能: 获取当前时间(YYYYMMDDHHSSMM)
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返 回 :   无
 ***************************************************/
extern void GetCurrentTime(char *sCurrentTime);

/***************************************************
*函数名称: CheckDate
*函数功能: 校验日期的合法性
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 不合法
 ***************************************************/
extern int CheckDate(char *sDate);


/*****************************************************
*函数名称: GetDateDiff
*函数功能: 计算两个日期之间间隔的天数
*输入参数: sStartDate -- 起始日期(YYYYMMDD)
            sEndDate   -- 终止日期(YYYYMMDD)
*输出参数: plDiffDays -- 两日期相隔天数
*函数返回: 0  -- 成功
            -1 -- 失败
*****************************************************/
extern int GetDateDiff(char *sStartDate, char *sEndDate, int *piDiffDays);

#endif


