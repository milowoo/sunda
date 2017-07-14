/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  ���ܣ�ʱ�������������
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo ����.
 *************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>

#define ___IsLeapYear(N) (((N % 4 == 0) && (N % 100 != 0)) || (N % 400 == 0)) /*�Ƿ�Ϊ����*/

#define YEARDAYS(a) ___IsLeapYear(a)?366:365      /*һ�������*/
#define MON2DAYS(a) ___IsLeapYear(a)?29:28        /*���µ�����*/

/*һ���µ�����*/
#define RMONTHDAYS(a, b) ((a) == 2?(MON2DAYS(b)):((a) < 8 && (a) % 2 == 0 || (a) >= 8 && (a) % 2 != 0)?30:31)

/***************************************************
*��������: GetDate
*��������: ��ȡ����ʱ��(YYYYMMDD)
*�������: sDate -- ���� yyyymmdd
*�������: ��
*����: 0 --- �Ϸ�
      -1 -- ���Ϸ�
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
*��������: GetCurrentTime
*��������: ��ȡ��ǰʱ��(YYYYMMDDHHSSMM)
*�������: sDate -- ���� yyyymmdd
*�������: ��
*����: 0 --- �Ϸ�
      -1 -- ���Ϸ�
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
*��������: CheckDate
*��������: У�����ڵĺϷ���
*�������: sDate -- ���� yyyymmdd
*�������: ��
*����: 0 --- �Ϸ�
      -1 -- ���Ϸ�
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
*��������: checkdate
*��������: У�����ڵĺϷ���
*�������: iYear -- �� iMonth -- �� iDay -- ��
*�������: ��
*����: 0 --- �Ϸ�
      -1 -- ���
      -2 -- �´�
      -3 -- �մ�
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
*��������: CountDate
*��������: ����һ�����ڼ��ϻ��߼�ȥһ�����������õ�����
*�������: lStartDate -- ��ʼ����(YYYYMMDD)
           iOffset    -- ƫ����(������ʾ��ȥ)
*�������: sResultDate -- ������������
*����ֵ:   0  -- �ɹ�
           -1 -- ʧ��
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
*��������: CheckStrDate
*��������: У�����ڵĺϷ���
*�������: sDate -- ���� yyyymmdd
*�������: ��
*����: 0 --- �Ϸ�
      -1 -- ���Ϸ�
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
*��������: GetDateDiff
*��������: ������������֮����������
*�������: sStartDate -- ��ʼ����(YYYYMMDD)
            sEndDate   -- ��ֹ����(YYYYMMDD)
*�������: plDiffDays -- �������������
*��������: 0  -- �ɹ�
            -1 -- ʧ��
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
*��������: DiffDate
*��������: ������������֮����������
*�������: lStartDate -- ��ʼ����(YYYYMMDD)
            lEndDate   -- ��ֹ����(YYYYMMDD)
*�������: piDiffDays -- �������������
*��������: 0  -- �ɹ�
            -1 -- ʧ��
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


/****************************************************** �ļ����� ****************************************/

