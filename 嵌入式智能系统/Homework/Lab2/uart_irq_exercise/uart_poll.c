/* uart_poll.c - UART0 轮询模式（参考 PPT 第六章 第187-195页）
 * 本文件是作业的参考框架，作业要求把 Uart_Getch / Uart_SendByte
 * 改造成中断驱动模式，见 uart_irq.c
 */

#include "2410addr.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define PCLK 50000000  /* 系统 PCLK 频率，50MHz，按实际情况修改 */

/* 软件延时，防止串口终端响应太慢导致数据丢失 */
static void Delay(int count)
{
    volatile int i;
    for (i = 0; i < count * 100; i++);
}

/* 初始化 UART0
 * pclk: 外部传入的 PCLK 频率，传 0 则使用宏定义的 PCLK
 * baud: 波特率，例如 115200
 */
void Uart_Init(int pclk, int baud)
{
    /* 配置 GPIO H 组：GPH2 复用为 TXD0，GPH3 复用为 RXD0 */
    rGPHCON |= 0xa0;
    /* 关闭 GPH2、GPH3 的内部上拉电阻 */
    rGPHUP   = 0x0;

    if (pclk == 0)
        pclk = PCLK;

    /* 关闭三路 UART 的 FIFO，使用简单的单字节收发模式 */
    rUFCON0 = 0x0;
    rUFCON1 = 0x0;
    rUFCON2 = 0x0;
    /* 关闭三路 UART 的自动流控（AFC） */
    rUMCON0 = 0x0;
    rUMCON1 = 0x0;
    rUMCON2 = 0x0;

    /* 线路控制寄存器：普通模式，无校验，1位停止位，8位数据位 */
    rULCON0 = 0x3;

    /* 控制寄存器：时钟源选 PCLK，TX/RX 均为轮询模式
     * bits[3:2]=01 -> RX 轮询/中断模式
     * bits[1:0]=01 -> TX 轮询/中断模式
     * 0x5 = 0b0000_0101
     */
    rUCON0  = 0x5;

    /* 波特率分频寄存器：公式 = PCLK / (16 * baud) - 1，加 0.5 四舍五入 */
    rUBRDIV0 = ((int)(pclk / 16 / baud + 0.5) - 1);
}

/* 轮询接收一个字节
 * CPU 在此死等，直到 RX 缓冲区有数据才返回
 */
char Uart_Getch(void)
{
    /* 轮询 UTRSTAT0 的 bit0：0=无数据，1=有数据 */
    while (!(rUTRSTAT0 & 0x1));

    /* 检查接收错误：bit0=溢出错误，bit2=帧错误 */
    if ((rUERSTAT0 & 0x1) || (rUERSTAT0 & 0x4))
        return -1;

    /* 从接收缓冲寄存器读出数据 */
    return RdURXH0;
}

/* 接收一个字符串，直到收到回车符 '\r' 为止 */
void Uart_GetString(char *string)
{
    char c;
    while ((c = Uart_Getch()) != '\r')
        *string++ = c;
    *string = '\0';  /* 字符串结尾补 null */
}

/* 接收一个字符串并解析为整数
 * 支持十进制（直接输入）、十六进制（0x 前缀或 H 后缀）、负数（- 前缀）
 */
int Uart_GetIntNum(void)
{
    char str[30];
    char *string = str;
    int base      = 10;  /* 默认十进制 */
    int minus     = 0;   /* 是否为负数 */
    int result    = 0;   /* 最终结果 */
    int lastIndex;       /* 字符串最后一个字符的下标 */
    int i;

    Uart_GetString(string);  /* 先接收字符串 */

    /* 检查负号 */
    if (string[0] == '-') {
        minus = 1;
        string++;
    }
    /* 检查 0x/0X 十六进制前缀 */
    if (string[0] == '0' && (string[1] == 'x' || string[1] == 'X')) {
        base    = 16;
        string += 2;
    }

    lastIndex = strlen(string) - 1;
    if (lastIndex < 0)
        return -1;

    /* 检查 h/H 十六进制后缀 */
    if (string[lastIndex] == 'h' || string[lastIndex] == 'H') {
        base = 16;
        string[lastIndex] = 0;
        lastIndex--;
    }

    if (base == 10) {
        /* 十进制直接用标准库 atoi */
        result = atoi(string);
        result = minus ? (-1 * result) : result;
    } else {
        /* 十六进制逐字符手动解析：每次左移4位（×16）再加当前位的值 */
        for (i = 0; i <= lastIndex; i++) {
            if (isalpha(string[i])) {
                if (isupper(string[i]))
                    result = (result << 4) + string[i] - 'A' + 10;
                else
                    result = (result << 4) + string[i] - 'a' + 10;
            } else {
                result = (result << 4) + string[i] - '0';
            }
        }
        result = minus ? (-1 * result) : result;
    }
    return result;
}

/* 轮询发送一个字节
 * CPU 在此死等，直到 TX 缓冲区为空才写入
 */
void Uart_SendByte(int data)
{
    /* 轮询 UTRSTAT0 的 bit2：0=TX缓冲区非空，1=TX缓冲区空可写 */
    while (!(rUTRSTAT0 & 0x4));
    Delay(10);       /* 等待终端软件跟上，避免丢字符 */
    WrUTXH0(data);   /* 写入发送缓冲寄存器，硬件自动发出 */
}

/* 发送一个以 null 结尾的字符串 */
void Uart_SendString(char *pt)
{
    while (*pt)
        Uart_SendByte(*pt++);
}
