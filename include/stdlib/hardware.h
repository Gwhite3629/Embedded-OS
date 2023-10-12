#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#define IO_BASE 0x80000000

/* * * * *
 * UART  *
 * * * * */

#include "uart.h"

/* * * *
 * USB *
 * * * */

#define USBCTRL0 0x80080000
#define USBCTRL0_S 0x10000

#define USBCTRL1 0x80090000
#define USBCTRL1_S 0x10000

/* * * * *
 * GPIO  *
 * * * * */



/* * * * * * * *
 * Interrupts  *
 * * * * * * * */

/* OFFSETS */

#define ICOLL_OFFSET    IO_BASE + 0x0

// Interrupt address offsets
#define ICOLL_VECTOR    0x0
#define ICOLL_LEVELACK  0x10
#define ICOLL_CTRL      0x20
#define ICOLL_VBASE     0x40
#define ICOLL_STAT      0x70

// Interrupt collector raw input registers
#define ICOLL_RAW0      0xA0
#define ICOLL_RAW1      0xB0
#define ICOLL_RAW2      0xC0
#define ICOLL_RAW3      0xD0

// Interrupt collector registers
#define ICOLL_INT_BASE  0x0120
#define ICOLL_INT_OFF   0x0010
#define ICOLL_INT_NUM   0x0080

// Interrupt collector debug register
#define ICOLL_DEBUG     0x1120
// Interrupt collector debug read register
#define ICOLL_DBGRD0    0x1130
#define ICOLL_DBGRD1    0x1140

// Interrupt debug flag register
#define ICOLL_DBGFLAG   0x1150

// Interrupt collector debug read request registers
#define ICOLL_DBGRQ0    0x1160
#define ICOLL_DBGRQ1    0x1170
#define ICOLL_DBGRQ2    0x1180
#define ICOLL_DBGRQ3    0x1190

// Interrupt collector version register
#define ICOLL_VERSION   0x11E0

// Common Offsets
#define ICOLL_SET    0x004
#define ICOLL_CLR    0x008
#define ICOLL_TOG    0x00C

// Priority levels
#define ICOLL_LVL0          0x0
#define ICOLL_LVL1          0x1
#define ICOLL_LVL2          0x2
#define ICOLL_LVL3          0x3

// Priority acknowledge
#define ICOLL_ACK_LVL0      0x1
#define ICOLL_ACK_LVL1      0x2
#define ICOLL_ACK_LVL2      0x4
#define ICOLL_ACK_LVL3      0x8

/* MASKS */

// Interrupt control vector
#define ICOLL_CTRL_SFTRST       0x1F    // Soft reset
#define ICOLL_CTRL_CLKGATE      0x1E    // Disables clocks
#define ICOLL_CTRL_VEC_PITCH    0x15    // Multiplier for interrupt register
#define ICOLL_CTRL_BYPASS_FSM   0x14    // Bypasses handshake
#define ICOLL_CTRL_NO_NESTING   0x13    // Disables int nesting
#define ICOLL_CTRL_ARM_RSE_M    0x12    // Enables arm style RSE
#define ICOLL_CTRL_FIQ_FIN_EN   0x11    // Sends FIQ to CPU
#define ICOLL_CTRL_IEQ_FIN_EN   0x10    // Sends IRQ to CPU

// Interrupt register
#define ICOLL_INTERRUPT_ENFIQ   0x04    // Sets to non-vec FIQ
#define ICOLL_INTERRUPT_SOFTIRQ 0x03    // Enforce software IRQ
#define ICOLL_INTERRUPT_ENABLE  0x02    // Enable bit
#define ICOLL_INTERRUPT_PRIO    0x00    // Priority bits

/* * * * *
 * TIMER *
 * * * * */

/* OFFSETS */

#define TIMROT_OFFSET       0x00068000

// Rotary deconder control register
#define TIMROT_ROTCTRL      0x00
// Rotary decoder up/down counter
#define TIMROT_ROTCOUNT     0x10
// Timer base registers
#define TIMROT_TIM0         0x20
#define TIMROT_TIM1         0x60
#define TIMROT_TIM2         0xA0
#define TIMROT_TIM3         0xE0

// Generic timer offsets
#define TIMROT_CTRL         0x00    // Timer control
#define TIMROT_RUNCOUNT     0x10    // Running count
#define TIMROT_FIXCOUNT     0x20    // Fixed count
#define TIMROT_MATCOUNT     0x30    // Match count

// Timer version register
#define TIMROT_VERSION      0x120

// Generic control offsets
#define TIMROT_SET      0x004
#define TIMROT_CLR      0x008
#define TIMROC_TOG      0x00C

/* MASKS */

// Rotary Control register
#define TIMROT_ROTCTRL_SFTRST   0x1F    // Forces a reset
#define TIMROT_ROTCTRL_CLKGATE  0x1E    // Gates off clocks
#define TIMROT_ROTCTRL_ROTPRES  0x1D    // Is rotary decoder present
#define TIMROT_ROTCTRL_TIM3PRE  0x1C    // Timer 3 is preset?
#define TIMROT_ROTCTRL_TIM2PRE  0x1B    // Timer 2 is preset?
#define TIMROT_ROTCTRL_TIM1PRE  0x1A    // Timer 1 is preset?
#define TIMROT_ROTCTRL_TIM0PRE  0x19    // Timer 0 is preset?
#define TIMROT_ROTCTRL_STATE    0x16    // Rotary decoder state
#define TIMROT_ROTCTRL_DIVIDER  0x10    // Clock divisor
#define TIMROT_ROTCTRL_REL      0x0C    // Resets rotary decoder on read
#define TIMROT_ROTCTRL_OVER     0x0A    // Oversample rate for decoder
#define TIMROT_ROTCTRL_POL_B    0x09    // Invert edge detector
#define TIMROT_ROTCTRL_POL_A    0x08    // Invert edge detector
#define TIMROT_ROTCTRL_SEL_B    0x04    // Select tick source
#define TIMROT_ROTCTRL_SEL_A    0x00    // Select tick source

// Timer control register
#define TIMROT_CTRL_IRQ         0x0F    // Sets when timer hits 0
#define TIMROT_CTRL_IEQ_EN      0x0E    // Enables interrupts
#define TIMROT_CTRL_MATCH_MODE  0x0B    // Timer match mode
#define TIMROT_CTRL_POLARITY    0x08    // Invert edge detector
#define TIMROT_CTRL_UPDATE      0x07    // Synchronous update
#define TIMROT_CTRL_RELOAD      0x06    // Reset on 0
#define TIMROT_CTRL_PRESCALE    0x04    // Divisor for clock
#define TIMROT_CTRL_SELECT      0x00    // Choose timer tick source

/* * * *
 * RNG *
 * * * */



/* * * *
 * PWM *
 * * * */



/* * * *
 * DMA *
 * * * */


#endif