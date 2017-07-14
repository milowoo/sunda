/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：时间操作公共函数
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo 建立.
 *************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>

#define ___IsLeapYear(N) (((N % 4 == 0) && (N % 100 != 0)) || (N % 400 == 0)) /*是否为闰年*/

#define YEARDAYS(a) ___IsLeapYear(a)?366:365      /*一年的天数*/
#define MON2DAYS(a) ___IsLeapYear(a)?29:28        /*二月的天数*/

/*一个月的天数*/
#define RMONTHDAYS(a, b) ((a) == 2?(MON2DAYS(b)):((a) < 8 && (a) % 2 == 0 || (a) >= 8 && (a) % 2 != 0)?30:31)

/***************************************************
*函数名称: GetDate
*函数功能: 获取当天时间(YYYYMMDD)
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 不合法
 ***************************************************/
char *GetDate( char * buff ) 
{
    time_t time(), now;
    struct tm * tTime, * localtime();

    time( &now );
    tTime = localtime( &now );

    tTime->tm_year = tTime->tm_year + 1900;

    sprintf( buff, "%.4d%.2d%.2d", tTime->tm_year,
        tTime->tm_mon+1, tTime->tm_mday );
    return buff;
}


/***************************************************
*函数名称: GetCurrentTime
*函数功能: 获取当前时间(YYYYMMDDHHSSMM)
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 不合法
 ***************************************************/
void GetCurrentTime(char *sCurrentTime)
{
   time_t current;
   struct tm *tmCurrentTime;

   tzset();
   time(&current);
   tmCurrentTime = localtime(&current);
   sprintf(sCurrentTime, "%4d%02d%02d%02d%02d%02d",
   tmCurrentTime->tm_year + 1900, tmCurrentTime->tm_mon + 1,
   tmCurrentTime->tm_mday, tmCurrentTime->tm_hour,
   tmCurrentTime->tm_min, tmCurrentTime->tm_sec);
}


/***************************************************
*函数名称: CheckDate
*函数功能: 校验日期的合法性
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 不合法
 ***************************************************/
int CheckDate(char *sDate)
{
    int iRet;

    char sYear[5];
    char sMonth[3];
    char sDay[3];

    memset(sYear,  0, sizeof(sYear));
    memset(sMonth, 0, sizeof(sMonth));
    memset(sDay,   0, sizeof(sDay));

    if (strlen(sDate) != 8)
    {
        return -1;
    }

    memcpy(sYear,  sDate, 4);
    memcpy(sMonth, sDate+4, 2);
    memcpy(sDay,   sDate+6, 2);

    iRet = checkdate(atoi(sYear), atoi(sMonth), atoi(sDay));
    if (iRet != 0)
    {
        return -1;
    }

    return 0;
}

/***************************************************
*函数名称: checkdate
*函数功能: 校验日期的合法性
*输入参数: iYear -- 年 iMonth -- 月 iDay -- 日
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 年错
      -2 -- 月错
      -3 -- 日错
 ***************************************************/
int checkdate(int iYear, int iMonth, int iDay)
{
    if (iYear < 0 || iYear > 9999)
        return -1;
    switch (iMonth)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        if (iDay <= 0 || iDay > 31)
        {
            return -3;
        }
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        if (iDay <= 0 || iDay > 30)
        {
            return -3;
        }
        break;
    case 2:
        if ((iYear % 4 == 0 && iYear % 100 != 0) || iYear % 400 == 0)
        {
            if (iDay <= 0 || iDay > 29)
            {
                return -3;
            }
        }
        else
        {
            if (iDay <= 0 || iDay > 28)
            {
                return -3;
            }
        }
        break;
    default:
        return -2;
    }
    return 0;
}

/*****************************************************
*函数名称: CountDate
*函数功能: 计算一个日期加上或者减去一个天数后所得的日期
*输入参数: lStartDate -- 起始日期(YYYYMMDD)
           iOffset    -- 偏移量(负数表示减去)
*输出参数: sResultDate -- 计算所得日期
*返回值:   0  -- 成功
           -1 -- 失败
*****************************************************/
long CountDate(char *sStartDate, int iOffset, char *sResultDate)
{
    int  step;
    long lStartDate;
    int  iYear, iMonth, iDays;
    long i;

    lStartDate  =  atol(sStartDate);

    iYear  = lStartDate / 10000;
    iMonth = lStartDate % 10000 / 100;
    iDays  = lStartDate % 100;

    if (checkdate(iYear, iMonth, iDays) != 0)
        return -1;

    step = (iOffset >= 0?1:-1);

    for(i = 1; i <= abs(iOffset); i ++)
    {
        iDays += step;
        if (iDays <= 0 || iDays > RMONTHDAYS(iMonth, iYear))
        {
            iMonth += step;
            if (iMonth <= 0)
            {
                iYear += step;
                iMonth = 12;
            }
            else if (iMonth > 12)
            {
                iYear += step;
                iMonth = 1;
            }

            if (iDays <= 0)
                iDays = RMONTHDAYS(iMonth, iYear);
            else
                iDays = 1;
        }
    }

    lStartDate = iYear * 10000 + iMonth * 100 + iDays;

    sprintf(sResultDate, "%8ld", lStartDate);

    return 0;
}




/***************************************************
*函数名称: CheckStrDate
*函数功能: 校验日期的合法性
*输入参数: sDate -- 日期 yyyymmdd
*输出参数: 无
*返回: 0 --- 合法
      -1 -- 不合法
 ***************************************************/
int CheckStrDate(char *sDate)
{
    int iRet;

    char sYear[5];
    char sMonth[3];
    char sDay[3];

    memset(sYear,  0, sizeof(sYear));
    memset(sMonth, 0, sizeof(sMonth));
    memset(sDay,   0, sizeof(sDay));

    if (strlen(sDate) != 8)
    {
        return -1;
    }

    memcpy(sYear,  sDate, 4);
    memcpy(sMonth, sDate+4, 2);
    memcpy(sDay,   sDate+6, 2);

    iRet = checkdate(atoi(sYear), atoi(sMonth), atoi(sDay));
    if (iRet != 0)
    {
        return -1;
    }

    return 0;
}


/*****************************************************
*函数名称: GetDateDiff
*函数功能: 计算两个日期之间间隔的天数
*输入参数: sStartDate -- 起始日期(YYYYMMDD)
            sEndDate   -- 终止日期(YYYYMMDD)
*输出参数: plDiffDays -- 两日期相隔天数
*函数返回: 0  -- 成功
            -1 -- 失败
*****************************************************/
int GetDateDiff(char *sStartDate, char *sEndDate, int *piDiffDays)
{
    int   iRet;
    long  lStartDate;
    long  lEndDate;

    lStartDate = atol(sStartDate);
    lEndDate   = atol(sEndDate);

    iRet = DiffDate(lStartDate, lEndDate, piDiffDays);
    if (iRet != 0)
    {
        return -1;
    }

    return 0;
}

/*****************************************************
*函数名称: DiffDate
*函数功能: 计算两个日期之间间隔的天数
*输入参数: lStartDate -- 起始日期(YYYYMMDD)
            lEndDate   -- 终止日期(YYYYMMDD)
*输出参数: piDiffDays -- 两日期相隔天数
*函数返回: 0  -- 成功
            -1 -- 失败
*****************************************************/
int DiffDate(long lStartDate, long lEndDate, long *piDiffDays)
{
    int  i, flag = 1;
    int  sYear, sMonth, sDays;
    int  eYear, eMonth, eDays;
    long tmp_date;
    long Days = 0;

    if (lStartDate > lEndDate)
    {
        tmp_date = lEndDate;
        lEndDate = lStartDate;
        lStartDate = tmp_date;
        flag = -1;
    }

    sYear = lStartDate / 10000;
    eYear = lEndDate / 10000;

    sMonth = lStartDate % 10000 / 100;
    eMonth = lEndDate % 10000 / 100;

    sDays = lStartDate % 100;
    eDays = lEndDate % 100;

    if (checkdate(sYear, sMonth, sDays) != 0 ||
        checkdate(eYear, eMonth, eDays) != 0)
        return -1;

    for(i = sYear; i < eYear; i ++)
        Days += YEARDAYS(i);

    for(i = 1; i < eMonth; i ++)
        Days += RMONTHDAYS(i, eYear);

    Days += eDays;

    for(i = 1; i < sMonth; i ++)
        Days -= RMONTHDAYS(i, sYear);

    Days -= sDays;
    *piDiffDays = flag * Days;

    return 0;
}


/****************************************************** 文件结束 ****************************************/

