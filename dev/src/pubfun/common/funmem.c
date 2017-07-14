/*********************************************************************
 *  Copyright 2012, by MiloWoo.
 *  All right reserved.
 *
 *  功能：IPC共享内存操作封装函数
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

int shm_shmget_internal(key_t shm_key, int shm_size, int flags) 
{
    int       shmid;
    
    if ((shmid = shmget(shm_key, shm_size, flags)) == -1) {
        return -1;
    }

    return shmid;
}

void *shm_attach_internal(int shmid) 
{
    char   *shm;

    shm = shmat(shmid, (void*)0, 0);
    if (shm ==  NULL) {
        return NULL;
    }
    
    return shm;
}

int shm_delete_internal(int shmid)
{
    struct shmid_ds shmds;

    if (shmctl(shmid, IPC_RMID, &shmds) < 0) {
        return -1;
    }

    return 0;
}


int shm_getid(const char *shmname, int shmsize, int createflg)
{
    int    flags;
    key_t  shmkey;

    if (shmname != NULL)
        shmkey = makekey(shmname);
    else
        shmkey = IPC_PRIVATE;

    if (createflg == 1) {
        flags = SHM_PERM|IPC_CREAT;
    } else {
        flags = SHM_PERM;
    }

    return shm_shmget_internal(shmkey, shmsize, flags);
}

void *shm_create(const char *shmname, int shmsize)
{
    int shmid;

    shmid = shm_getid(shmname, shmsize, 1);
    if (shmid < 0) {
        return NULL;
    }

    return (void *)shm_attach_internal(shmid);
}

void *shm_connect(const char *shmname)
{
    int shmid;

    shmid = shm_getid(shmname, 0, 0);
    if (shmid < 0) {
        return NULL;
    }

    return (void *)shm_attach_internal(shmid);
}

int shm_delete(const char *shmname, const char *mem)
{
    struct shmid_ds shmds;
    int    memid;

    if (mem != NULL && shmdt(mem) == -1) {
        return -1;
    }

    memid = shm_getid(shmname, 0, 0);
    if (memid < 0) 
        return -1;

    return shm_delete_internal(memid);
}


/************************************************ 文件结束 *********************************/

