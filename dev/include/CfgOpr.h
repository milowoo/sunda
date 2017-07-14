#ifndef _CFG_H_
#define _CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_PARAM_CFG   getenv("SYS_PARAM_CFG")

extern FILE* cf_open(const char *);
extern int cf_close(FILE*);
extern int cf_nextline(FILE* , char *, int);
extern int cf_rewind(FILE*);
extern int cf_locate(FILE* , const char *, char *);
extern int cf_locatenum(FILE*, const char *, int *);
extern int cf_nextparm(FILE*, const char *, char *); 

#ifdef __cplusplus
}
#endif
 
#endif
