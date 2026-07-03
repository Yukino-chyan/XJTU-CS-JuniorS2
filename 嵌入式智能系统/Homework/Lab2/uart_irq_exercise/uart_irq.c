#include "2410addr.h"
#include "2410lib.h"   /* EnableIRQ() */
#include "option.h"    /* pISR_UART0, PCLK */
#include <string.h>    /* strlen() */

/* TX 发送缓冲区，用于中断驱动发送 */
static const char    *tx_buf   = NULL;
static volatile int   tx_idx   = 0;
static int            tx_total = 0;

/* -----------------------------------------------------------------------
 * Uart_Init_IRQ
 * 初始化 UART0 为中断模式 + loopback
 * 与轮询版本的区别：
 *   rUCON0 bit[5]=1   -> loopback 模式
 *   rUCON0 bit[3:2]=01 -> TX 中断模式
 *   rUCON0 bit[1:0]=01 -> RX 中断模式
 * 另增加：清挂起寄存器、打开收发中断屏蔽
 * ----------------------------------------------------------------------- */
void Uart_Init_IRQ(int pclk, int baud)
{
    rGPHCON &= ~0xF0;
    rGPHCON |= 0xa0;    /* GPH2->TXD0，GPH3->RXD0 */
    rGPHUP   = 0x0;
    if (pclk == 0) pclk = PCLK;
    rUFCON0 = 0x0;      /* 关闭 FIFO */
    rUMCON0 = 0x0;      /* 关闭 AFC */
    rULCON0 = 0x3;      /* 8位数据，1停止位，无校验 */
    rUCON0 = 0x25;
    rUBRDIV0 = ((int)(pclk / 16 / baud + 0.5) - 1);
    /* 清除三个挂起寄存器，防止残留中断 */
    rSUBSRCPND |= (BIT_SUB_TXD0 | BIT_SUB_RXD0);          /* 清 SUBSRCPND[1:0]（TXD0、RXD0） */
    rSRCPND    |= BIT_UART0;    /* 清 SRCPND[28] */
    rINTPND    |= BIT_UART0;    /* 清 INTPND[28] */
    /* 打开总中断屏蔽和收发子中断屏蔽 */
    rINTMOD    =  0x0;
    rINTMSK    &= ~BIT_UART0;
    rINTSUBMSK &= ~(BIT_SUB_TXD0 | BIT_SUB_RXD0);
}

/* -----------------------------------------------------------------------
 * RX 子程序：接收中断处理
 * loopback 模式下收到的是自己发出去的字符，此处读出即可
 * ----------------------------------------------------------------------- */
static void Uart_RX_Handler(void)
{
    char c = RdURXH0;   /* 读出接收缓冲区字符 */
    (void)c;            /* loopback 下收到的是自己发的，不需要额外处理 */
}

/* -----------------------------------------------------------------------
 * TX 子程序：发送中断处理
 * TX 缓冲区空时触发，发送下一个字符；发完则关闭 TX 中断
 * ----------------------------------------------------------------------- */
static void Uart_TX_Handler(void)
{
    if (tx_buf != NULL && tx_idx < tx_total) {
        WrUTXH0(tx_buf[tx_idx++]);     /* 发下一个字符 */
    } else {
        rINTSUBMSK |= BIT_SUB_TXD0;   /* 发完，关闭 TX 子中断 */
    }
}

/* -----------------------------------------------------------------------
 * UART0 中断服务程序
 * 进入时先屏蔽收发中断，根据 UTRSTAT 分别调用 RX/TX 子程序，
 * 清挂起寄存器后取消屏蔽，等待下一次中断
 * ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
 * 硬件 IRQ 总入口：读 INTOFFSET，从软件向量表取出对应 ISR 并调用
 * vectors.s 中 IRQ 向量指向此函数
 * ----------------------------------------------------------------------- */
void __irq IRQ_Handler(void)
{
    typedef void (*isr_t)(void);
    /* 软件向量表基址 0x33FFFF24，每项 4 字节，INTOFFSET 给出中断号 */
    isr_t isr = (isr_t)(*(volatile unsigned *)(0x33FFFF24 + rINTOFFSET * 4));
    isr();
}

/* UART0 中断服务程序（由 IRQ_Handler 通过软件向量表调用，非硬件直接进入） */
void UART0_ISR(void)
{
    rINTSUBMSK |= (BIT_SUB_TXD0 | BIT_SUB_RXD0);//屏蔽收发中断
    //根据 UTRSTAT 判断并调用对应子程序
    if (rUTRSTAT0 & 0x1) Uart_RX_Handler();
    if (rUTRSTAT0 & 0x2) Uart_TX_Handler();
    /*清除挂起寄存器，顺序：SUBSRCPND -> SRCPND -> INTPND */
    rSUBSRCPND |= (BIT_SUB_TXD0 | BIT_SUB_RXD0);
    rSRCPND    |= BIT_UART0;
    rINTPND    |= BIT_UART0;
    /*取消中断屏蔽，等待下一次中断 */
    rINTSUBMSK &= ~BIT_SUB_RXD0;
    if (tx_buf != NULL && tx_idx < tx_total)
        rINTSUBMSK &= ~BIT_SUB_TXD0;
}

/* -----------------------------------------------------------------------
 * 中断方式发送字符串
 * 设置全局发送缓冲区，手动发第一个字符触发 TX 中断链，
 * 后续字符由 TX 中断驱动逐个发出
 * ----------------------------------------------------------------------- */
void Uart_SendString_IRQ(const char *str)
{
    if (str == NULL || str[0] == '\0') return;  /* 空字符串保护 */
    tx_buf   = str;
    tx_total = strlen(str);
    tx_idx   = 0;
    WrUTXH0(tx_buf[tx_idx++]);   /* 发第一个字符，触发TX缓冲区空中断 */
}

/* -----------------------------------------------------------------------
 * main
 * 顺序：初始化 -> 注册ISR -> 开全局中断 -> 发送字符串（复位程序最后）
 * ----------------------------------------------------------------------- */
int main(void)
{
    Uart_Init_IRQ(0, 115200);
    pISR_UART0 = (unsigned)UART0_ISR;
    EnableIRQ();                       
    Uart_SendString_IRQ("以中断方式\r\n");
    while (1) { }
    return 0;
}
