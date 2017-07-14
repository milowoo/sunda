#ifndef _TASK_H_
#define _TASK_H_

#define	TSK_TYPE_PERM  1
#define	TSK_TYPE_TEMP  2
#define TSK_FLAGS_WAIT 0x00000001

#define TSK_CMD_START      1
#define TSK_CMD_EXIT       2
#define TSK_CMD_FRESH      3


typedef struct _tskcmd_t {
    int  cmd;          /* command to perform */
    int  id;           /* server id for new task */
    int  err;          /* error code, 0 success others fail */
} tskcmd_t;

#endif
