#ifndef __2410LIB_H__
#define __2410LIB_H__

__inline void EnableIRQ(void)
{
    int tmp;
    __asm
    {
        MRS tmp, CPSR
        BIC tmp, tmp, #0x80
        MSR CPSR_c, tmp
    }
}
#endif
