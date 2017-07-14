/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：文件操作函数
 *
 *  Edit History:
 *
 *    2012/03/17 - MiloWoo 建立.
 *************************************************************************/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>


FILE* cf_open(const char *file)
{
    FILE *fpcfg = NULL;

    fpcfg = fopen(file, "r");
    if (fpcfg  == NULL) {
        printf("fopen: [%s] error: [%d]\n", file, errno);
        return NULL;
    }

    return fpcfg;
}

int cf_close(FILE* fpcfg)
{
    if (fpcfg != NULL)
        fclose(fpcfg);

    return 0;
}

int cf_nextline(FILE* fpcfg, char *bp, int size)
{
    if (fpcfg == NULL)  return -1;
    fgets(bp, size, fpcfg);
    AllTrim(bp);
    return 0;
}

int cf_rewind(FILE* fpcfg)
{
    if (fpcfg == NULL)  return -1;
    fseek(fpcfg, 0L, SEEK_SET);
    return 0;
}

int cf_locate(FILE* fpcfg, const char *pname, char *value)
{
    int   len;
    char  line[512 + 1];

    if (fpcfg == NULL) return -1;

    cf_rewind(fpcfg);
    while (!feof(fpcfg)) {
        memset(line, 0, sizeof(line));
        cf_nextline(fpcfg, line, sizeof(line));
        if (line[0] == '#') continue;
        if ((line[strlen(pname)] == 0x00 || line[strlen(pname)] == 0x20) &&
            !memcmp(line, pname, strlen(pname))) {
            break;
        }
    }

    if (feof(fpcfg)) return -1;

    strcpy(value, &line[strlen(pname)]);
    AllTrim(value);
    return 0;
}

int cf_locatenum(FILE* fpcfg, const char *pname, int *value)
{
    char    buf[24 + 1];

    if (fpcfg == NULL) return -1;

    memset(buf, 0, sizeof(buf));
    if (cf_locate(fpcfg, pname, buf) < 0) return -1;
    if (!IsDigitBuf(buf, strlen(buf))) return -1;

    *value = atoi(buf);
    return 0;
}

int cf_nextparm(FILE* fpcfg, const char *pname, char *value)
{
    char  line[512 + 1];

    if (fpcfg == NULL) return -1;

    while (!feof(fpcfg)) {
        memset(line, 0, sizeof(line));
        cf_nextline(fpcfg, line, sizeof(line));
        if (line[0] == '#') continue;
        if ((line[strlen(pname)] == 0x00 || line[strlen(pname)] == 0x20) &&
            !memcmp(line, pname, strlen(pname))) {
            break;
        }
    }

    if (feof(fpcfg))
        return -1;

    LeftTrim(&line[strlen(pname)]);
    RightTrim(line);
    strcpy(value, &line[strlen(pname)]);
    return 0;
}

