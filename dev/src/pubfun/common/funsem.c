/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：IPC信号操作封装函数
 *
 *  Edit History:
 *
 *    2012/03/12 - MiloWoo 建立.
 *************************************************************************/

#include "IpcOpr.h"

static key_t makekey(const char *name)
{
    key_t   key, base_key;
    int     i;
    char    *stop;
    char    buf[128 + 1];
        
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < strlen(name); i++) {
        sprintf(buf + strlen(buf), "%02x", name[i]);
    }

    key = strtol(buf, &stop, 16);
    if (getenv("IPCREGION") == NULL)
        base_key = 0;
    else
        base_key = atoi(getenv("IPCREGION"));
    if (base_key < 0 || base_key > 255) base_key = 0;
    
    key = key + IPC_KEY_BASE * base_key;
    return key;
}

int sem_semget_internal(key_t sem_key, int sem_size, int flags)
{
    int   sem_id;

    /*  create the semaphore    */
    if ((sem_id = semget(sem_key, sem_size, flags)) == -1) {
        return -1;
    }

    return sem_id;
}

int sem_attach_internal(int sem_id, int sem_size)
{
    int   i;
    union semum {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } arg;

    /*  set the semaphore initial value(s)      */
    arg.val = 1;
    for (i = 0; i < sem_size; i++) {
        if (semctl(sem_id, i, SETVAL, arg) == -1) {
            return -1;
        }  /* end if */
    } /* end for */

    return sem_id;
}

int sem_delete_internal(int sem_id)
{
    /*  delete the semaphore */
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        return (-1);
    }

    return (0);
}

/* ipc operation function */
int sem_getid(const char *semname, int semsize, int createflg)
{
    int    flags;
    key_t  semkey;

    if (semname != NULL)
        semkey = makekey(semname);
    else
        semkey = IPC_PRIVATE;

    if (createflg == 1) {
        flags = SEM_PERM|IPC_CREAT;
    } else {
        flags = SEM_PERM;
    }

    return sem_semget_internal(semkey, semsize, flags);
}

static int sem_call(int sem_num, int sem_id, int op)
{
    struct sembuf SemBuf;

    SemBuf.sem_num = sem_num;
    SemBuf.sem_op = op;
    SemBuf.sem_flg = SEM_UNDO;

    if (semop(sem_id, &SemBuf, 1) < 0) {
        return(-1);
    }

    return (0);
}

int sem_create(const char *semname, int semsize)
{
    int semid;

    semid = sem_getid(semname, semsize, 1);
    if (semid < 0) {
        return -1;
    }

    return sem_attach_internal(semid, semsize);
}

int sem_delete(const char *semname, int semsize)
{
    int semid;

    semid = sem_getid(semname, semsize, 0);
    if (semid < 0) {
        return -1;
    }
    
    return sem_delete_internal(semid);
}

int sem_connect(const char *semname, int semsize)
{
    int semid;

    semid = sem_getid(semname, semsize, 0);
    if (semid < 0) {
        return -1;
    }

    return sem_attach_internal(semid, semsize);
}

int sem_lock(int sem_num, int sem_id)
{
    /*  lock the specified semaphore */
    if (sem_call(sem_num, sem_id, -1) == -1) {
        return (-1);
    }

    return (0);
}

int sem_unlock(int sem_num, int sem_id)
{
    /*  operate the specified semaphore */
    if (sem_call(sem_num, sem_id, 1) == -1) {
        return (-1);
    }

    return (0);
}

/*********************************************** 文件结束 *********************************************/

