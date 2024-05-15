#include "../../include/stdlib/uart.h"
#include "../../include/stdlib/string.h"

volatile uint32_t *devs[5] = {
    UARTAPP0,
    UARTAPP1,
    UARTAPP2,
    UARTAPP3,
    UARTAPP4
};

int open_UART(const char *dev, uint32_t baud)
{
    int dnum = -1;
    uint32_t *addr = 0x0;
    uint32_t divisor;

    // Get uart device from user
    if (!strncmp("UART0", dev, 6)) {
        dnum = 0;
    } else if (!strncmp("UART1", dev, 6)) {
        dnum = 1;
    } else if (!strncmp("UART2", dev, 6)) {
        dnum = 2;
    } else if (!strncmp("UART3", dev, 6)) {
        dnum = 3;
    } else if (!strncmp("UART4", dev, 6)) {
        dnum = 4;
    } else {
        return -E_BADARG;
    }

    // Get base address
    addr = devs[dnum];

    // Confirm device is present
    if (!(*(addr + UART_STAT) & (0b1 << UART_STAT_PRESENT))) {
        return -E_NODEV;
    }

    // Calculate 22-bit baud rate divisor
    divisor = (UARTCLK * 32) / baud;

    /* * * * * * * * * * * * *
     * UART control register *
     * * * * * * * * * * * * */

    // Enable UART
    *(addr + UART_CTRL2) &= (0b0 << UART_CTRL2_UARTEN);
    *(addr + UART_CTRL2) |= (0b1 << UART_CTRL2_UARTEN);

    // Enable RX and TX
    *(addr + UART_CTRL2) &= (0b0 << UART_CTRL2_RXE);
    *(addr + UART_CTRL2) &= (0b0 << UART_CTRL2_TXE);
    *(addr + UART_CTRL2) |= (0b1 << UART_CTRL2_RXE);
    *(addr + UART_CTRL2) |= (0b1 << UART_CTRL2_TXE);


    /* * * * * * * * * * * * * *
     * Line control register 1 *
     * * * * * * * * * * * * * */

    // Set divisor
    *(addr + UART_LCTL1) &= (0x0000 << UART_LCTL1_BAUD_DIVINT);
    *(addr + UART_LCTL1) |= ((divisor & 0xFFFF) << UART_LCTL1_BAUD_DIVINT);
    *(addr + UART_LCTL1) &= (0b000000 << UART_LCTL1_BAUD_DIVFRAC);
    *(addr + UART_LCTL1) |= ((divisor & 0b111111) << UART_LCTL1_BAUD_DIVFRAC);

    // Set word length to 8-bits
    *(addr + UART_LCTL1) &= (0b00 << UART_LCTL1_WLEN);
    *(addr + UART_LCTL1) |= (0b11 << UART_LCTL1_WLEN);

    // Set parity to off
    *(addr + UART_LCTL1) &= (0b0 << UART_LCTL1_STP2);
    *(addr + UART_LCTL1) &= (0b0 << UART_LCTL1_EPS);
    *(addr + UART_LCTL1) &= (0b0 << UART_LCTL1_PEN);

    return dnum;
}

int send_UART(int dev, char *buf, size_t num)
{
    int s = 0;
    uint32_t *addr = 0x0;

    if (num < 1) {
        return num;
    }

    addr = devs[dev];

    while (num > 0) {
        if (!(*(addr + UART_STAT) & (0b1 << UART_STAT_BUSY))) {
            if (!((*addr + UART_STAT) & (0b1 << UART_STAT_TXFF))) {
                *(addr + UART_DATA) = buf[s];
                s++;
                num--;
            }
        }
        if (!(*(addr + UART_STAT) & (0b1111 << UART_STAT_FERR))) {
            return (0x0030 + ((*(addr + UART_STAT) & (0b11 << UART_STAT_FERR)) >> UART_STAT_FERR));
        }
    }

    return s;
}

int read_UART(int dev, char *buf, size_t num)
{
    int r = 0;
    uint32_t *addr = 0x0;

    if (num < 1) {
        return num;
    }

    addr = devs[dev];

    while (num > 0) {
        if (!(*(addr + UART_STAT) & (0b1 << UART_STAT_BUSY))) {
            if (!((*addr + UART_STAT) & (0b1 << UART_STAT_TXFF))) {
                buf[r] = *(addr + UART_DATA);
                r++;
                num--;
            }
        }
        if (!(*(addr + UART_STAT) & (0b1111 << UART_STAT_FERR))) {
            return (0x0030 + ((*(addr + UART_STAT) & (0b11 << UART_STAT_FERR)) >> UART_STAT_FERR));
        }
    }

    return r;
}