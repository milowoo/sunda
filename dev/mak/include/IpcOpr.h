#ifndef _IPC_H_
#define _IPC_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHM_PERM 0660
#define SEM_PERM 0660
#define MSQ_PERM 0660
#define IPC_KEY_BASE  0x1010101

extern int   sem_create(const char *, int);
extern int   sem_connect(const char *, int);
extern int   sem_delete(const char *, int);
extern int   sem_lock(int, int);
extern int   sem_unlock(int, int);
extern int   msq_create(const char *);
extern int   msq_connect(const char *);
extern int   msq_delete(const char *);
extern int   msq_write(int, char *, int, long, int);
extern int   msq_read(int, char *, int, long, int);
extern void *shm_create(const char *, int);
extern void *shm_connect(const char *);
extern int   shm_delete(const char *, const char *);

extern int   shm_shmget_internal(key_t, int, int);
extern void *shm_attach_internal(int);
extern int   shm_delete_internal(int);
extern int   sem_semget_internal(key_t, int, int);
extern int   sem_attach_internal(int, int);
extern int   sem_delete_internal(int);
extern int   msq_msqget_internal(int, int);
extern int   msq_delete_internal(int);

extern int   sem_getid(const char *semname, int semsize, int createflg);
extern int   shm_getid(const char *shmname, int shmsize, int createflg);
extern int   msq_getid(const char *quename, int createflg);


#ifdef __cplusplus
}
#endif

#endif
