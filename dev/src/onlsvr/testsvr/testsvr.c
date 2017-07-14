#include "sunda.h"

extern testsvr(TPSVCINFO *trans);


tpsvrinit(int argc,char ** argv)
{

    if (tpadvertise("testsvr", testsvr)) 
    {
        SysLog(SYS_TRACE_NORMAL,"tpadvertise error %d %s", tperrno, tpstrerror(tperrno));
        return (-1);
    }

    SysLog(SYS_TRACE_ERROR,"=======================server start========================");
    return 0;
}

void tpsvrdone()
{

    SysLog(SYS_TRACE_ERROR,"=======================server exit========================");
    return;
}


/* 标准借贷记 */
testsvr(TPSVCINFO *trans)
{
    int  rc;

   // SysLog(SYS_TRACE_NORMAL,"=== begin ===");
    
    trans->len = 20;
	memcpy(trans->data, "test222222 111111112", trans->len);
    /* 交易处理结束, 返回 */
    tpreturn(TPSUCCESS, 0, trans->data, trans->len, 0);
}
