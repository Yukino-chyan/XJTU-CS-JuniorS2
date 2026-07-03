/* 2410addr.h - S3C2440 UART Register Definitions (from PPT Chapter 6) */

#ifndef __2410ADDR_H__
#define __2410ADDR_H__

/* ========== UART 0 ========== */
#define rULCON0   (*(volatile unsigned *)0x50000000)  // UART 0 Line control
#define rUCON0    (*(volatile unsigned *)0x50000004)  // UART 0 Control
#define rUFCON0   (*(volatile unsigned *)0x50000008)  // UART 0 FIFO control
#define rUMCON0   (*(volatile unsigned *)0x5000000c)  // UART 0 Modem control
#define rUTRSTAT0 (*(volatile unsigned *)0x50000010)  // UART 0 Tx/Rx status
#define rUERSTAT0 (*(volatile unsigned *)0x50000014)  // UART 0 Rx error status
#define rUFSTAT0  (*(volatile unsigned *)0x50000018)  // UART 0 FIFO status
#define rUMSTAT0  (*(volatile unsigned *)0x5000001c)  // UART 0 Modem status
#define rUBRDIV0  (*(volatile unsigned *)0x50000028)  // UART 0 Baud rate divisor

/* ========== UART 1 ========== */
#define rULCON1   (*(volatile unsigned *)0x50004000)  // UART 1 Line control
#define rUCON1    (*(volatile unsigned *)0x50004004)  // UART 1 Control
#define rUFCON1   (*(volatile unsigned *)0x50004008)  // UART 1 FIFO control
#define rUMCON1   (*(volatile unsigned *)0x5000400c)  // UART 1 Modem control
#define rUTRSTAT1 (*(volatile unsigned *)0x50004010)  // UART 1 Tx/Rx status
#define rUERSTAT1 (*(volatile unsigned *)0x50004014)  // UART 1 Rx error status
#define rUFSTAT1  (*(volatile unsigned *)0x50004018)  // UART 1 FIFO status
#define rUMSTAT1  (*(volatile unsigned *)0x5000401c)  // UART 1 Modem status
#define rUBRDIV1  (*(volatile unsigned *)0x50004028)  // UART 1 Baud rate divisor

/* ========== UART 2 ========== */
#define rULCON2   (*(volatile unsigned *)0x50008000)  // UART 2 Line control
#define rUCON2    (*(volatile unsigned *)0x50008004)  // UART 2 Control
#define rUFCON2   (*(volatile unsigned *)0x50008008)  // UART 2 FIFO control
#define rUMCON2   (*(volatile unsigned *)0x5000800c)  // UART 2 Modem control
#define rUTRSTAT2 (*(volatile unsigned *)0x50008010)  // UART 2 Tx/Rx status
#define rUERSTAT2 (*(volatile unsigned *)0x50008014)  // UART 2 Rx error status
#define rUFSTAT2  (*(volatile unsigned *)0x50008018)  // UART 2 FIFO status
#define rUMSTAT2  (*(volatile unsigned *)0x5000801c)  // UART 2 Modem status
#define rUBRDIV2  (*(volatile unsigned *)0x50008028)  // UART 2 Baud rate divisor

/* ========== UART TX/RX Buffers (Little Endian) ========== */
#define rUTXH0  (*(volatile unsigned char *)0x50000020)  // UART 0 Transmission Hold
#define rURXH0  (*(volatile unsigned char *)0x50000024)  // UART 0 Receive buffer
#define rUTXH1  (*(volatile unsigned char *)0x50004020)  // UART 1 Transmission Hold
#define rURXH1  (*(volatile unsigned char *)0x50004024)  // UART 1 Receive buffer
#define rUTXH2  (*(volatile unsigned char *)0x50008020)  // UART 2 Transmission Hold
#define rURXH2  (*(volatile unsigned char *)0x50008024)  // UART 2 Receive buffer

#define WrUTXH0(ch) (*(volatile unsigned char *)0x50000020) = (unsigned char)(ch)
#define RdURXH0     (*(volatile unsigned char *)0x50000024)
#define WrUTXH1(ch) (*(volatile unsigned char *)0x50004020) = (unsigned char)(ch)
#define RdURXH1()   (*(volatile unsigned char *)0x50004024)
#define WrUTXH2(ch) (*(volatile unsigned char *)0x50008020) = (unsigned char)(ch)
#define RdURXH2()   (*(volatile unsigned char *)0x50008024)

/* DMA byte-access addresses */
#define UTXH0  (0x50000020)
#define URXH0  (0x50000024)
#define UTXH1  (0x50004020)
#define URXH1  (0x50004024)
#define UTXH2  (0x50008020)
#define URXH2  (0x50008024)

/* ========== GPIO ========== */
#define rGPHCON (*(volatile unsigned *)0x56000070)  // Port H control
#define rGPHUP  (*(volatile unsigned *)0x56000078)  // Port H pull-up control

/* ========== Interrupt Controller ========== */
#define rSRCPND  (*(volatile unsigned *)0x4A000000)  // Interrupt request status
#define rINTMOD  (*(volatile unsigned *)0x4A000004)  // Interrupt mode (IRQ/FIQ)
#define rINTMSK  (*(volatile unsigned *)0x4A000008)  // Interrupt mask
#define rINTPND  (*(volatile unsigned *)0x4A000010)  // Interrupt pending
#define rINTOFFSET (*(volatile unsigned *)0x4A000014) // Interrupt offset
#define rSUBSRCPND (*(volatile unsigned *)0x4A000018) // Sub source pending
#define rINTSUBMSK (*(volatile unsigned *)0x4A00001C) // Sub mask

/* Interrupt bits */
#define BIT_UART0   (1 << 28)
#define BIT_SUB_RXD0 (1 << 0)
#define BIT_SUB_TXD0 (1 << 1)
#define BIT_SUB_ERR0 (1 << 2)

#endif /* __2410ADDR_H__ */
