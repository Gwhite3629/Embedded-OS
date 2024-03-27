#ifndef _UART_H_
#define _UART_H_

#include "err.h"

#define UARTCLK 0x016E3600 // 24 MHz

// Debug UART register locations
#define UART_DEBUG_DR   ((volatile uint32_t *) 0x80074000)  // Data register
#define UART_DEBUG_ECR  ((volatile uint32_t *) 0x80074004)  // Status Read / Error Clear
#define UART_DEBUG_FR   ((volatile uint32_t *) 0x80074018)  // Flag Register
#define UART_DEBUG_ILPR ((volatile uint32_t *) 0x80074020)  // Low-Power counter register
#define UART_DEBUG_IBRD ((volatile uint32_t *) 0x80074024)  // Integer baud divisor
#define UART_DEBUG_FBRD ((volatile uint32_t *) 0x80074028)  // Fractional baud divisor
#define UART_DEBUG_H    ((volatile uint32_t *) 0x8007402C)  // Line control, high
#define UART_DEBUG_CR   ((volatile uint32_t *) 0x80074030)  // Control
#define UART_DEBUG_IFLS ((volatile uint32_t *) 0x80074034)  // Interrupt FIFO select
#define UART_DEBUG_IMSC ((volatile uint32_t *) 0x80074038)  // Interrupt mask set / clear
#define UART_DEBUG_RIS  ((volatile uint32_t *) 0x8007403C)  // Raw interrupt status register
#define UART_DEBUG_MIS  ((volatile uint32_t *) 0x80074040)  // Masked interrupt status register
#define UART_DEBUG_ICR  ((volatile uint32_t *) 0x80074044)  // Interrupt clear register
#define UART_DEBUG_DMAC ((volatile uint32_t *) 0x80074048)  // DMA control

// Debug UART data masks
#define DUART_DR_OE     0xB         // Overrun error
#define DUART_DR_BE     0xA         // Break error
#define DUART_DR_PE     0x9         // Parity error
#define DUART_DR_FE     0x8         // Framing error
#define DUART_DR_D      0x0         // Data (8 bits)

// Debug UART status masks
#define DUART_ECR_EC    0x4         // Error clear (4 bits)
#define DUART_ECR_OE    0x3         // Overrun error
#define DUART_ECR_BE    0x2         // Break error
#define DUART_ECR_PE    0x1         // Parity error
#define DUART_ECR_FE    0x0         // Framing error

// Debug UART flag masks
#define DUART_FR_RI     0x8         // Ring indicator
#define DUART_FR_TXFE   0x7         // Transmit FIFO empty
#define DUART_FR_RXFF   0x6         // Receive FIFO full
#define DUART_FR_TXFF   0x5         // Transmit FIFO full
#define DUART_FR_RXFE   0x4         // Receive FIFI empty
#define DUART_FR_BUSY   0x3         // UART busy
#define DUART_FR_DCD    0x2         // Data carrier detect
#define DUART_FR_DSR    0x1         // Data set ready
#define DUART_FR_CTS    0x0         // Clear to send

// Debug UART low-power counter mask
#define DUART_ILPR_DVSR 0x0         // Low power divisor (8 bits)

// Debug UART integer baud rate divisor mask
#define DUART_IBRD_DIVI 0x0         // Baud rate divisor (16 bits)

// Debug UART fractional baud rate divisor masks
#define DUART_FBRD_DIVF 0x0         // Baud rate divisor (6 bits)

// Debug UART line control register masks, high byte
#define DUART_H_SPS     0x7         // Stick parity select
#define DUART_H_WLEN    0x5         // Word length (2 bits)
                                    // (11 = 8 bits)
                                    // (10 = 7 bits)
                                    // (01 = 6 bits)
                                    // (00 = 5 bits)
#define DUART_H_FEN     0x4         // Enable FIFO
#define DUART_H_STP2    0x3         // Two stop bits select
#define DUART_H_EPS     0x2         // Even parity select
#define DUART_H_PEN     0x1         // Parity enable
#define DUART_H_BRK     0x0         // Send break

// Debug UART control register masks
#define DUART_CR_CTSEN  0xF         // CTS hardware flow ctrl
#define DUART_CR_RTSEN  0xE         // RTS hardware flow ctrl
#define DUART_CR_OUT2   0xD         // Not implemented
#define DUART_CR_OUT1   0xC         // Not implemented
#define DUART_CR_RTS    0xB         // Request to send
#define DUART_CR_DTR    0xA         // Data transmit ready, not implemented
#define DUART_CR_RXE    0x9         // Receive enable
#define DUART_CR_TXE    0x8         // Transmit enable
#define DUART_CR_LBE    0x7         // Loop back enable
#define DUART_CR_SIRLP  0x2         // SIR low power mode, not supported
#define DUART_CR_SIREN  0x1         // SIR enable, not supported
#define DUART_CR_UARTEN 0x0         // UART enable

// Debug UART interrupt FIFO level select
#define DUART_IFLS_RXSEL 0x3         // Receive interrupts FIFO level select (3 bits)
                                    // (000 = 1/8) Trigger at 1/8 FIFO full
                                    // (001 = 1/4) Trigger at 1/4 FIFO full
                                    // (010 = 1/2) Trigger at 1/2 FIFO full
                                    // (011 = 3/4) Trigger at 3/4 FIFO full
                                    // (100 = 7/8) Trigger at 7/8 FIFO full
                                    // (101 = INVALID) Reserved
                                    // (110 = INVALID) Reserved
                                    // (111 = INVALID) Reserved
#define DUART_IFLS_TXSEL 0x0         // Transmit interrupts FIFO level select (3 bits)
                                    // (000 = 1/8) Trigger at 1/8 FIFO full
                                    // (001 = 1/4) Trigger at 1/4 FIFO full
                                    // (010 = 1/2) Trigger at 1/2 FIFO full
                                    // (011 = 3/4) Trigger at 3/4 FIFO full
                                    // (100 = 7/8) Trigger at 7/8 FIFO full
                                    // (101 = INVALID) Reserved
                                    // (110 = INVALID) Reserved
                                    // (111 = INVALID) Reserved

// Debug UART interrupts mask set/clear register masks
#define DUART_IMSC_OEIM 0xA         // Overrun error interrupt mask
#define DUART_IMSC_BEIM 0x9         // Break error interrupt mask
#define DUART_IMSC_PEIM 0x8         // Parity error interrupt mask
#define DUART_IMSC_FEIM 0x7         // Framing error interrupt mask
#define DUART_IMSC_RTIM 0x6         // Receive timeout interrupt mask
#define DUART_IMSC_TXIM 0x5         // Transmit interrupt mask
#define DUART_IMSC_RXIM 0x4         // Receive interrupt mask
#define DUART_IMSC_DSRM 0x3         // DSR modem interrupt mask
#define DUART_IMSC_DCDM 0x2         // DCD modem interrupt mask
#define DUART_IMSC_CTSM 0x1         // CTS modem interrupt mask
#define DUART_IMSC_RIM  0x0         // RI modem interrupt mask

// Debug UART raw interrupt status register
#define DUART_RIS_OERIS 0xA         // Overrun error interrupt status
#define DUART_RIS_BERIS 0x9         // Break error interrupt status     
#define DUART_RIS_PERIS 0x8         // Parity error interrupt status
#define DUART_RIS_FERIS 0x7         // Framing error interrupt status
#define DUART_RIS_RTRIS 0x6         // Receive timeout interrupt status
#define DUART_RIS_TXRIS 0x5         // Transmit interrupt status
#define DUART_RIS_RXRIS 0x4         // Receive interrupt status
#define DUART_RIS_RRMIS 0x3         // DSR modem interrupt status
#define DUART_RIS_DRMIS 0x2         // DCD modem interrupt status
#define DUART_RIS_SRMIS 0x1         // CTS modem interrupt status
#define DUART_RIS_IRMIS 0x0         // RI modem interrupt status

// Debug UART masked interrupt status register
#define DUART_MIS_OERIS 0xA         // Overrun error interrupt status
#define DUART_MIS_BERIS 0x9         // Break error interrupt status     
#define DUART_MIS_PERIS 0x8         // Parity error interrupt status
#define DUART_MIS_FERIS 0x7         // Framing error interrupt status
#define DUART_MIS_RTRIS 0x6         // Receive timeout interrupt status
#define DUART_MIS_TXRIS 0x5         // Transmit interrupt status
#define DUART_MIS_RXRIS 0x4         // Receive interrupt status
#define DUART_MIS_RRMIS 0x3         // DSR modem interrupt status
#define DUART_MIS_DRMIS 0x2         // DCD modem interrupt status
#define DUART_MIS_SRMIS 0x1         // CTS modem interrupt status
#define DUART_MIS_IRMIS 0x0         // RI modem interrupt status

// Debug UART interrupt clear register
#define DUART_ICR_OEIC  0xA         // Overrun error interrupt clear
#define DUART_ICR_BEIC  0x9         // Break error interrupt clear
#define DUART_ICR_PEIC  0x8         // Parity error interrupt clear
#define DUART_ICR_FEIC  0x7         // Framing error interrupt clear
#define DUART_ICR_RTIC  0x6         // Receive timeout interrupt clear
#define DUART_ICR_TXIC  0x5         // Transmit interrupt clear
#define DUART_ICR_RXIC  0x4         // Receive interrupt clear
#define DUART_ICR_DSRMIC 0x3         // DSR modem interrupt clear
#define DUART_ICR_DCDMIC 0x2         // DCD modem interrupt clear
#define DUART_ICR_CTSMIC 0x1         // CTS modem interrupt clear
#define DUART_ICR_RIMIC 0x0         // RI modem interrupt clear

// Debug UART DMA control register (All reserved)
#define DUART_DMAC_DMAON 0x2         // Reserved
#define DUART_DMAC_TXDMA 0x1         // Reserved
#define DUART_DMAC_RXDMA 0x0         // Reserved

void init_debug(void);

// Application UART 0
#define UARTAPP0        ((volatile uint32_t *) 0x8006A000)

// Application UART 1
#define UARTAPP1        ((volatile uint32_t *) 0x8006C000)

// Application UART 2
#define UARTAPP2        ((volatile uint32_t *) 0x8006E000)

// Application UART 3
#define UARTAPP3        ((volatile uint32_t *) 0x80070000)

// Application UART 4
#define UARTAPP4        ((volatile uint32_t *) 0x80072000)

volatile uint32_t *devs[5] = {
    UARTAPP0,
    UARTAPP1,
    UARTAPP2,
    UARTAPP3,
    UARTAPP4
};

// Application UART register locations (Base addresses for fields)
#define UART_CTRL0      0x00        // Receive DMA control register
#define UART_CTRL1      0x10        // Transmit DMA control register
#define UART_CTRL2      0x20        // Control reguster
#define UART_LCTL1      0x30        // Line control register 1
#define UART_LCTL2      0x40        // Line control register 2
#define UART_INTR       0x50        // Interrupt register
#define UART_DATA       0x60        // Data register
#define UART_STAT       0x70        // Status register
#define UART_DEBUG      0x80        // Debug register
#define UART_VER        0x90        // Version register
#define UART_AUTOB      0xA0        // Autobaud register

// Application UART offsets (Address offsets for fields)
#define UART_BASE       0x000       // Base address word
#define UART_SET        0x004       // Set address word
#define UART_CLR        0x008       // Clear address word
#define UART_TOG        0x00C       // Toggle address word

/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * All following are masks into the address words  *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

// Application UART register for DMA controls and receiving data
#define UART_CTRL0_SFTRST       0x1F    // Hold in reset state
#define UART_CTRL0_CLKGATE      0x1E    // Blocks gate level clocks
#define UART_CTRL0_RUN          0x1D    // Execute RX DMA, cleared on end of execution
#define UART_CTRL0_RX_SOURCE    0x1C    // Set RX source, either DMA(1) or RX data(0)
#define UART_CTRL0_RXTO_ENABLE  0x1B    // Set RX timeout to affect DMA
#define UART_CTRL0_RXTIMEOUT    0x10    // 8-bit time to wait for RX timeout (11-bits)
#define UART_CTRL0_XFER_COUNT   0x00    // Number of bytes to receive (multiples of 4)

// Application UART register for DMA controls and transmitting data
#define UART_CTRL1_RSVD2        0x1D    // Reserved (3 bits)
#define UART_CTRL1_RUN          0x1C    // Execute TX DMA, cleared on end of execution
#define UART_CTRL1_RSVD1        0x10    // Reserved (12 bits)
#define UART_CTRL1_XFER_COUNT   0x00    // Number of bytes to transmit (multiples of 4)

// Application UART register for FIFO select and DMA control
#define UART_CTRL2_INVERT_RTS   0x1F    // Invert RTS signal before transmitting
#define UART_CTRL2_INVERT_CTS   0x1E    // Invert CTS signal before sampling
#define UART_CTRL2_INVERT_TX    0x1D    // Invert TX signal before transmitting
#define UART_CTRL2_INVERT_RX    0x1C    // Invert RX signal before sampling
#define UART_CTRL2_RTS_SEMAPHORE 0x1B   // Deassert RTS when semaphore threshold < 2
#define UART_CTRL2_DMAONERR     0x1A    // DMA will terminate on error
#define UART_CTRL2_TXDMAE       0x19    // TX DMA enable
#define UART_CTRL2_RXDMAE       0x18    // RX DMA enable
#define UART_CTRL2_RSVD2        0x17    // Reserved
#define UART_CTRL2_RXIFLSEL     0x14    // Receive interrupt FIFO level select (3 bits)
                                        // (000 = ANY) FIFO contains at least 2 entries
                                        // (001 = 1/4) FIFO contains at least 4 entries
                                        // (010 = 1/2) FIFO contains at least 8 entries
                                        // (011 = 3/4) FIFO contains at least 12 entries
                                        // (100 = 7/8) FIFO contains at least 14 entries
                                        // (101 = INVALID) Reserved
                                        // (110 = INVALID) Reserved
                                        // (111 = INVALID) Reserved
#define UART_CTRL2_RSVD3        0x13    // Reserved
#define UART_CTRL2_TXIFLSEL     0x10    // Transmit interrupt FIFO level select (3 bits)
                                        // (000 = EMPTY) FIFO contains LEQ 2 entries
                                        // (001 = 1/4) FIFO contains LEQ 4 entries
                                        // (010 = 1/2) FIFO contains LEQ 8 entries
                                        // (011 = 3/4) FIFO contains LEQ 12 entries
                                        // (100 = 7/8) FIFO contains LEQ 14 entries
                                        // (101 = INVALID) Reserved
                                        // (110 = INVALID) Reserved
                                        // (111 = INVALID) Reserved
#define UART_CTRL2_CTSEN        0x0F    // Hardware flow control enable CTS
#define UART_CTRL2_RTSEN        0x0E    // Hardware flow control enable RTS
#define UART_CTRL2_OUT2         0x0D    // Not supported
#define UART_CTRL2_OUT1         0x0C    // Not supported
#define UART_CTRL2_RTS          0x0B    // Manually control RTS
#define UART_CTRL2_DTR          0x0A    // Not supported
#define UART_CTRL2_RXE          0x09    // Enable RX
#define UART_CTRL2_TXE          0x08    // Enable TX
#define UART_CTRL2_LBE          0x07    // Enable loop back
#define UART_CTRL2_USE_LCE2     0x06    // Use Line control 2
#define UART_CTRL2_RSVD4        0x03    // Reserved
#define UART_CTRL2_SIRLP        0x02    // Unsupported
#define UART_CTRL2_SIREN        0x01    // Unsupported
#define UART_CTRL2_UARTEN       0x00    // UART enable

// Application UART register for line control 1
#define UART_LCTL1_BAUD_DIVINT  0x10    // Baud rate integer (16-bit)
#define UART_LCTL1_RSVD         0x0E    // Reserved
#define UART_LCTL1_BAUD_DIVFRAC 0x08    // Baud rate fraction (6 bits)
#define UART_LCTL1_SPS          0x07    // Stick parity select
#define UART_LCTL1_WLEN         0x05    // Word length (2 bits)
#define UART_LCTL1_FEN          0x04    // Enable FIFO
#define UART_LCTL1_STP2         0x03    // Two stop bit select
#define UART_LCTL1_EPS          0x02    // Even parity select
#define UART_LCTL1_PEN          0x01    // Parity enable
#define UART_LCTL1_BRK          0x00    // Send break

// Application UART register for line control 2
#define UART_LCTL2_BAUD_DIVINT  0x10    // Baud rate integer (16-bit)
#define UART_LCTL2_RSVD         0x0E    // Reserved
#define UART_LCTL2_BAUD_DIVFRAC 0x08    // Baud rate fraction (6 bits)
#define UART_LCTL2_SPS          0x07    // Stick parity select
#define UART_LCTL2_WLEN         0x05    // Word length (2 bits)
#define UART_LCTL2_FEN          0x04    // Enable FIFO
#define UART_LCTL2_STP2         0x03    // Two stop bit select
#define UART_LCTL2_EPS          0x02    // Even parity select
#define UART_LCTL2_PEN          0x01    // Parity enable
#define UART_LCTL2_RSVD1        0x00    // Reserved

// Application UART register for interrupts
#define UART_INTR_RSVD1         0x1C    // Reserved (4-bits)
#define UART_INTR_ABDIEN        0x1B    // Automatic baudrate detected interrupt enable
#define UART_INTR_OEIEN         0x1A    // Overrun error interrupt enable
#define UART_INTR_BEIEN         0x19    // Break error interrupt enable
#define UART_INTR_PEIEN         0x18    // Parity error interrupt enable
#define UART_INTR_FEIEN         0x17    // Framing error interrupt enable
#define UART_INTR_RTIEN         0x16    // Receive timeout interrupt enable
#define UART_INTR_TXIEN         0x15    // Transmit interrupt enable
#define UART_INTR_RXIEN         0x14    // Receive interrupt enable
#define UART_INTR_DSRMIEN       0x13    // Not supported enable
#define UART_INTR_DCDMIEN       0x12    // Not supported enable
#define UART_INTR_CTSMIEN       0x11    // CTS modem interrupt enable
#define UART_INTR_RIMIEN        0x10    // RI modem interrupt enable
#define UART_INTR_RSVD2         0x0C    // Reserved
#define UART_INTR_ABDIS         0x0B    // Automatic baudrate detected interrupt status
#define UART_INTR_OEIS          0x0A    // Overrun error interrupt status
#define UART_INTR_BEIS          0x09    // Break error interrupt status
#define UART_INTR_PEIS          0x08    // Parity error interrupt status
#define UART_INTR_FEIS          0x07    // Framing error interrupt status
#define UART_INTR_RTIS          0x06    // Receive timeout interrupt status
#define UART_INTR_TXIS          0x05    // Transmit interrupt status
#define UART_INTR_RXIS          0x04    // Receive interrupt status
#define UART_INTR_DSRMIS        0x03    // Not supported
#define UART_INTR_DCDMIS        0x02    // Not supported
#define UART_INTR_CTSMIS        0x01    // CRS modem interrupt status
#define UART_INTR_RIMIS         0x00    // Not supproted

// Application UART data register
#define UART_DATA               0x00 // Whole word is data

// Application UART status register
#define UART_STAT_PRESENT       0x1F    // Read only, indicates if APP UART is present
#define UART_STAT_HISPEED       0x1E    // Read only, indicates if high speed is present
#define UART_STAT_BUSY          0x1D    // UART busy
#define UART_STAT_CTS           0x1C    // Clear to send
#define UART_STAT_TXFE          0x1B    // Transmit FIFO empty
#define UART_STAT_RXFF          0x1A    // Receive FIFO full
#define UART_STAT_TXFF          0x19    // Transmit FIFO full
#define UART_STAT_RXFE          0x18    // Receive FIFO empty
#define UART_STAT_RXBTYE_INVALID 0x14   // Invalid bits for RX data (4-bits)
#define UART_STAT_OERR          0x13    // Overrun error
#define UART_STAT_BERR          0x12    // Break error
#define UART_STAT_PERR          0x11    // Parity error
#define UART_STAT_FERR          0x10    // Framing error
#define UART_STAT_RXCOUNT       0x00    // Number of bytes received on an RX DMA

// Application UART debug, contains DMA signals
#define UART_DEBUG_RXIBAUD_DIV  0x10    // RX baud integer divisor (16-bits)
#define UART_DEBUG_RXFBAUD_DIV  0x0A    // RX baud fractional divisor (6-bits)
#define UART_DEBUG_RSVD1        0x06    // Reserved (4-bits)
#define UART_DEBUG_TXDMARUN     0x05    // DMA CMD run status TX
#define UART_DEBUG_RXDMARUN     0x04    // DMA CMD run status RX
#define UART_DEBUG_TXCMDEND     0x03    // DMA CMD end status TX
#define UART_DEBUG_RXCMDEND     0x02    // DMA CMD end status RX
#define UART_DEBUG_TXDMARQ      0x01    // DMA request status TX
#define UART_DEBUG_RXDMARQ      0x00    // DMA request status RX

// Application UART version register
#define UART_VERS_MAJOR         0x18    // Read only major field of RTL version (8-bits)
#define UART_VERS_MINOR         0x10    // Read only minor field of RTL version (8-bits)
#define UART_VERS_STEP          0x00    // Read only value for step of RTL version (16-bits)

// Application UART autobaud field values
#define UART_AUTOB_REFCHAR1     0x18    // Second reference char in baud rate detection (8-bits)
#define UART_AUTOB_REFCHAR0     0x10    // First reference char in baud rate detection (8-bits)
#define UART_AUTOB_RSVD1        0x05    // Reserved
#define UART_AUTOB_UPDATE_TX    0x04    // Set baud rate divisor
#define UART_AUTOB_TWO_REF_CHARS 0x03   // Set 2 baud rate detection chars
#define UART_AUTOB_START_WITH_RUNBIT 0x02 // Assert autobaud logic
#define UART_AUTOB_START_BAUD_DETECT 0x01 // Run autobaud logic
#define UART_AUTOB_BAUD_DETECT_ENABLE 0x00 // Enable autobaud

int open_UART(const char *dev, uint32_t baud);

int send_UART(int dev, char *buf, size_t num);

int read_UART(int dev, char *buf, size_t num);

#endif // _UART_H_