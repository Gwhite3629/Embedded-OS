#ifndef _HARDWARE_RPI4B_H_
#define _HARDWARE_RPI4B_H_

#define CASTA(X) ((X))
#define CASTV(X) ((X))

#define IO_BASE CASTA(0xfe000000)
#define MEM_SIZE CASTA(0x200000000)

/* Values from the BCM2835-ARM-Peripherals.pdf manual */
/* Also check the BCM2711 Peripherals manual */

/* All values will have IO_BASE added to them */
/* however that changes due to Pi Model */
/* so we handle that elsewhere */

/********/
/* GPIO */
/********/

#define GPIO_BASE	IO_BASE + CASTA(0x200000)

/* GPFSEL = Function Select Registers */
/*   Each reg 3 groups of 10 */
/*   000 = Input, 001 = Output */
/*   100=ALT0, 101=ALT1, 110=ALT2, 111=ALT3 011=ALT4, 010=ALT5 */
#define GPIO_GPFSEL0	CASTA(GPIO_BASE + 0x00)
#define GPIO_GPFSEL1	CASTA(GPIO_BASE + 0x04)
#define GPIO_GPFSEL2	CASTA(GPIO_BASE + 0x08)
#define GPIO_GPFSEL3	CASTA(GPIO_BASE + 0x0c)
#define GPIO_GPFSEL4	CASTA(GPIO_BASE + 0x10)
#define GPIO_GPFSEL5	CASTA(GPIO_BASE + 0x14)
/* GPSET = GPIO Set */
/*  SET0 to set GPIO 32-0  */
/*  SET1 to set GPIO 54-33 */
#define GPIO_GPSET0	CASTA(GPIO_BASE + 0x1c)
#define GPIO_GPSET1	CASTA(GPIO_BASE + 0x20)
/* GPCLR = GPIO Clear */
/*  CLR0 to clear GPIO 32-0  */
/*  CLR1 to clear GPIO 54-33 */
#define GPIO_GPCLR0	CASTA(GPIO_BASE + 0x28)
#define GPIO_GPCLR1	CASTA(GPIO_BASE + 0x2c)
/* GPLEV0 = Level Detect */
/*  Return actual value on the pins */
#define GPIO_GPLEV0	CASTA(GPIO_BASE + 0x34)
#define GPIO_GPLEV1	CASTA(GPIO_BASE + 0x38)
/* GPEDS = Event Detect */
/*  Set based on rising/falling event status */
/*  Can trigger an interrupt */
/*  Three interrupts; one for each bank plus one for "any" */
#define GPIO_GPEDS0	CASTA(GPIO_BASE + 0x40)
#define GPIO_GPEDS1	CASTA(GPIO_BASE + 0x44)
/* GPREN = Rising Edge Detect */
#define GPIO_GPREN0	CASTA(GPIO_BASE + 0x4c)
#define GPIO_GPREN1	CASTA(GPIO_BASE + 0x50)
/* GPFEN = Falling Edge Detect */
#define GPIO_GPFEN0	CASTA(GPIO_BASE + 0x58)
#define GPIO_GPFEN1	CASTA(GPIO_BASE + 0x5c)
/* GPHEN = High level detect */
#define GPIO_GPHEN0	CASTA(GPIO_BASE + 0x64)
#define GPIO_GPHEN1	CASTA(GPIO_BASE + 0x68)
/* GPHEN = Low level detect */
#define GPIO_GPLEN0	CASTA(GPIO_BASE + 0x70)
#define GPIO_GPLEN1	CASTA(GPIO_BASE + 0x74)
/* AREN = Asynchronous Rising Edge Detect */
#define GPIO_GPAREN0	CASTA(GPIO_BASE + 0x7c)
#define GPIO_GPAREN1	CASTA(GPIO_BASE + 0x80)
/* AREN = Asynchronous Falling Edge Detect */
#define GPIO_GPAFEN0	CASTA(GPIO_BASE + 0x88)
#define GPIO_GPAFEN1	CASTA(GPIO_BASE + 0x8c)
/* GPPUD = Pull Up/Down */
/* Set in conjunction with below */
/*   00 = disable, 01 = PullDown, 10 = PullUp */
#define GPIO_GPPUD	CASTA(GPIO_BASE + 0x94)
#define GPIO_GPPUD_DISABLE	CASTV(0x0)
#define GPIO_GPPUD_PULLDOWN	CASTV(0x1)
#define GPIO_GPPUD_PULLUP	CASTV(0x2)
/* To set the pullups: */
/* 1. Write to GPPUD to set the required control signal */
/*    Pull-up, Pull-Down, none                          */
/* 2. Wait 150 cycles (this provides required set-up time for signal */
/* 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads */
/*    you wish to modify (only pads which receive a clock will be modified) */
/* 4. Wait 150 cycles (this provides the required hold time) */
/* 5. Write to GPPUD to remove the control signal */
/* 6. Write to GPPUDCLK0/1 to remove the clock */
#define GPIO_GPPUDCLK0	CASTA(GPIO_BASE + 0x98)
#define GPIO_GPPUDCLK1	CASTA(GPIO_BASE + 0x9c)


/**************/
/* Interrupts */
/**************/
/* Section 7 */

/* Note Pi4 does things differently */

			/*7e00b000 */
#define IRQ_BASE	IO_BASE + CASTA(0xb000)


/* Pi1/Pi2/Pi3 */

#define IRQ_BASIC_PENDING	CASTA(IRQ_BASE+0x200)
#define IRQ_PENDING1		CASTA(IRQ_BASE+0x204)
#define IRQ_PENDING2		CASTA(IRQ_BASE+0x208)
#define IRQ_FIQ_CONTROL		CASTA(IRQ_BASE+0x20c)
#define IRQ_ENABLE_IRQ1		CASTA(IRQ_BASE+0x210)
#define IRQ_ENABLE_IRQ2		CASTA(IRQ_BASE+0x214)
#define IRQ_ENABLE_BASIC_IRQ	CASTA(IRQ_BASE+0x218)

#define IRQ_DISABLE_IRQ1	CASTA(IRQ_BASE+0x21c)
#define IRQ_DISABLE_IRQ2	CASTA(IRQ_BASE+0x220)
#define IRQ_DISABLE_BASIC_IRQ	CASTA(IRQ_BASE+0x224)


/* for Pi4, p100 of BCM2711 manual */

#define IRQ0_PENDING0		CASTA(IRQ_BASE+0x200)
#define IRQ0_PENDING1		CASTA(IRQ_BASE+0x204)
#define IRQ0_PENDING2		CASTA(IRQ_BASE+0x208)

#define IRQ0_SET_EN_0		CASTA(IRQ_BASE+0x210)
#define IRQ0_SET_EN_1		CASTA(IRQ_BASE+0x214)
#define IRQ0_SET_EN_2		CASTA(IRQ_BASE+0x218)

#define IRQ0_CLR_EN_0		CASTA(IRQ_BASE+0x220)
#define IRQ0_CLR_EN_1		CASTA(IRQ_BASE+0x224)
#define IRQ0_CLR_EN_2		CASTA(IRQ_BASE+0x228)

#define IRQ0_STATUS0		CASTA(IRQ_BASE+0x230)
#define IRQ0_STATUS1		CASTA(IRQ_BASE+0x234)
#define IRQ0_STATUS2		CASTA(IRQ_BASE+0x238)


/* FIXME: update this with Pi4 values */

#define IRQ_ENABLE_BASIC_IRQ_ACCESS_ERROR0	CASTV(1<<7)
#define IRQ_ENABLE_BASIC_IRQ_ACCESS_ERROR1	CASTV(1<<6)
#define IRQ_ENABLE_BASIC_IRQ_GPU1_HALTED	CASTV(1<<5)
#define IRQ_ENABLE_BASIC_IRQ_GPU0_HALTED	CASTV(1<<4)
#define IRQ_ENABLE_BASIC_IRQ_ARM_DOORBELL1	CASTV(1<<3)
#define IRQ_ENABLE_BASIC_IRQ_ARM_DOORBELL0	CASTV(1<<2)
#define IRQ_ENABLE_BASIC_IRQ_ARM_MAILBOX	CASTV(1<<1)
#define IRQ_ENABLE_BASIC_IRQ_ARM_TIMER		CASTV(1<<0)





/*************/
/* ARM Timer */
/*************/
/* Section 12 */
			/*7e00b000 */
#define TIMER_BASE	IO_BASE + CASTA(0xb000)
/* LOAD = Value to Count Down from */
#define TIMER_LOAD	CASTA(TIMER_BASE+0x400)
/* VALUE = Current timer Value */
#define TIMER_VALUE	CASTA(TIMER_BASE+0x404)
/* CONTROL = Control Values */
#define TIMER_CONTROL	CASTA(TIMER_BASE+0x408)
#define TIMER_CONTROL_FREE_ENABLE	CASTV(1<<9)	/* Enable Free Counter */
#define TIMER_CONTROL_HALT		CASTV(1<<8)	/* Halt if processor halted */
#define TIMER_CONTROL_ENABLE		CASTV(1<<7)	/* Enable counter */
				/* 6 = ignored, always free-running */
#define TIMER_CONTROL_INT_ENABLE	CASTV(1<<5)	/* Enable interrupt */
#define TIMER_CONTROL_PRESCALE_1	CASTV(0<<2)
#define TIMER_CONTROL_PRESCALE_16	CASTV(1<<2)
#define TIMER_CONTROL_PRESCALE_256	CASTV(2<<2)
#define TIMER_CONTROL_32BIT		CASTV(1<<1)	/* Manual says 23bit? typo? */
				/* 0 = ignored, always wrapping */

#define TIMER_IRQ_CLEAR	CASTA(TIMER_BASE+0x40c)
#define TIMER_RAW_IRQ	CASTA(TIMER_BASE+0x410)
#define TIMER_MASKED_IRQ	CASTA(TIMER_BASE+0x414)
#define TIMER_RELOAD	CASTA(TIMER_BASE+0x418)
#define TIMER_PREDIVIDER	CASTA(TIMER_BASE+0x41c)
#define TIMER_FREE_RUNNING	CASTA(TIMER_BASE+0x420)

/****************/
/* System Timer */
/****************/

#define SYS_TIMER_BASE IO_BASE + CASTA(0x3000)
// Control / Status register
#define SYS_TIMER_CS            CASTA(TIMER_BASE + 0x00)
#define SYS_TIMER_CS_M0         CASTV(1<<0)
#define SYS_TIMER_CS_M1         CASTV(1<<1)
#define SYS_TIMER_CS_M2         CASTV(1<<2)
#define SYS_TIMER_CS_M3         CASTV(1<<3)
// Counter Lower 32
#define SYS_TIMER_CLO           CASTA(TIMER_BASE + 0x04)

// Counter Higher 32
#define SYS_TIMER_CHI           CASTA(TIMER_BASE + 0x08)

#define SYS_TIMER_RATE          1000000

// Compare 0
#define SYS_TIMER_C0            CASTA(TIMER_BASE + 0x0c)

// Compare 1
#define SYS_TIMER_C1            CASTA(TIMER_BASE + 0x10)

// Compare 2
#define SYS_TIMER_C2            CASTA(TIMER_BASE + 0x14)

// Compare 3
#define SYS_TIMER_C3            CASTA(TIMER_BASE + 0x18)

/********/
/* UART */
/********/
			/*7e00b000 */
			/*7e201000 */
#define UART0_BASE	IO_BASE + CASTA(0x201000)
/* DR = Data register */
/* On write, write 8-bits to send */
/* On receive, get 12-bits:       */
/*    Overrun Error == Overflowed FIFO */
/*    Break Error == data held low over full time */
/*    Parity Error */
/*    Frame Error == missing stop bit */
/* Then 8 bits of data */
#define UART0_DR	CASTA(UART0_BASE + 0x00)
/* RSRECR = Receive Status Register */
/*  Has same 4 bits of error as in DR */
#define UART0_RSRECR	CASTA(UART0_BASE + 0x04)
/* FR = Flags Register */
#define UART0_FR	CASTA(UART0_BASE + 0x18)
#define UART0_FR_TXFE	CASTV(1<<7)	/* TXFE = Transmit FIFO empty */
#define UART0_FR_RXFF	CASTV(1<<6)	/* RXFF = Receive FIFO full   */
#define UART0_FR_TXFF	CASTV(1<<5)	/* TXFF = Transmit FIFO full */
#define UART0_FR_RXFE	CASTV(1<<4)	/* RXFE = Receive FIFO empty */
#define UART0_FR_BUSY	CASTV(1<<3)	/* BUSY = UART is busy transmitting */
				/*  2 = DCD  = Unsupported */
				/*  1 = DSR  = Unsupported */
#define UART0_FR_CTS	CASTV(1<<0)	/* CTS  = inverted version of nUARTCTS value */

/* ILPR = Infrared, disabled on BC2835 */
#define UART0_ILPR	CASTA(UART0_BASE + 0x20)
/* IBRD = Integer part of Baud Rate Divisor */
/*    bottom 16 bits */
/*    UART must be disabled to change */
#define UART0_IBRD	CASTA(UART0_BASE + 0x24)
/* FBRD = Fractional part of Baud Rate Divisor */
/*   bottom 5 bits */
/*   Baud rate divisor BAUDDIV = (FUARTCLK/(16 Baud rate)) */
/*    UART must be disabled to change */
#define UART0_FBRD	CASTA(UART0_BASE + 0x28)
/* LCRH = Line Control Register */
#define UART0_LCRH	CASTA(UART0_BASE + 0x2C)
#define UART0_LCRH_SPS	CASTV(1<<7)	/* SPS = Stick Parity Select (0=disabled) */
				/* WLEN = word length 00=5, 01=6, 10=7, 11=8 */
#define UART0_LCRH_WLEN_5BIT	CASTV(0<<5)
#define UART0_LCRH_WLEN_6BIT	CASTV(1<<5)
#define UART0_LCRH_WLEN_7BIT	CASTV(2<<5)
#define UART0_LCRH_WLEN_8BIT	CASTV(3<<5)
#define UART0_LCRH_FEN	CASTV(1<<4)	/* FEN = enable FIFOs */
#define UART0_LCRH_STP2	CASTV(1<<3)	/* STP2 = enable 2 stop bits */
#define UART0_LCRH_EPS	CASTV(1<<2)	/* EPS  = even parity select */
#define UART0_LCRH_PEN	CASTV(1<<1)	/* PEN  = parity enable */
#define UART0_LCRH_BRK	CASTV(1<<0)	/* BRK  = send break after next character */
/* CR = Control Register */
/*   To enable transmission TXE and UARTEN must be set to 1 */
/*   To enable reception RXE and UARTEN must be set to 1 */
/*   Program the control registers as follows: */
/*    1. Disable the UART.                     */
/*    2. Wait for the end of transmission or   */
/*       reception of the current character.   */
/*    3. Flush the transmit FIFO by setting    */
/*       the FEN bit to 0 in the Line Control  */
/*       Register, UART_LCRH.                  */
/*    4. Reprogram UART_CR                     */
/*    5. Enable the UART.                      */
#define UART0_CR	CASTA(UART0_BASE + 0x30)
#define UART0_CR_CTSEN	CASTV(1<<15)	/*   15 = CTSEN = CTS Flow Enable */
#define UART0_CR_RTSEN	CASTV(1<<14)	/*   14 = RTSEN = RTS Flow Enable */
				/*   13 = OUT2 = Unsupported */
				/*   12 = OUT1 = Unsupported */
#define UART0_CR_RTS	CASTV(1<<11)	/*   11 = RTS = Request to Send */
					/*   10 = DTR = Unsupported */
#define UART0_CR_RXE	CASTV(1<<9)	/*    9 = RXE = Receive Enable */
#define UART0_CR_TXE	CASTV(1<<8)	/*    8 = TXE = Transmit Enable */
#define UART0_CR_LBE	CASTV(1<<7)	/*    7 = LBE = Loopback Enable */
				/*    6 - 3 = RESERVED */
				/*    2 = SIRLP = Unsupported */
				/*    1 = SIREN = Unsupported */
#define UART0_CR_UARTEN	CASTV(1<<0)	/*    0 = UARTEN = UART Enable */
/* IFLS = FIFO Level Select */
/*  11 - 9 = RXIFPSEL = Unsupported */
/*   8 - 6 = TXIFPSEL = Unsupported */
/*   5 - 3 = RXIFLSEL = 000=1/8, 001=1/4, 010=1/2, 011=3/4 100=7/8 */
/*   2 - 0 = TXIFLSEL = 000=1/8, 001=1/4, 010=1/2, 011=3/4 100=7/8 */
#define UART0_IFLS	CASTA(UART0_BASE + 0x34)
/* IMSRC = Interrupt Mask Set/Clear */
/*   10 = OEIM = Overrun Interrupt Mask */
/*    9 = BEIM = Break Interrupt Mask */
/*    8 = PEIM = Parity Interrupt Mask */
/*    7 = FEIM = Framing Interrupt Mask */
/*    6 = RTIM = Receivce Timeout Mask */
/*    5 = TXIM = Transmit Interrupt Mask */
/*    4 = RXIM = Receive Masked Interrupt Mask */
/*    3 = DSRMIM (unsupported) */
/*    2 = DCDMIM (unsupported) */
/*    1 = CTSMIM = nUARTCTS Mask */
/*    0 = RIMIM (unsupported) */
#define UART0_IMSC	CASTA(UART0_BASE + 0x38)
/* RIS = Raw Interrupt Status */
#define UART0_IMSC_OE	CASTV(1<<10) /* OERIS = Overrun Interrupt Raw Status */
#define UART0_IMSC_BE	CASTV(1<<9)  /* BERIS = Break Interrupt Raw Status*/
#define UART0_IMSC_PE	CASTV(1<<8)  /* PERIS = Parity Interrupt Raw Status */
#define UART0_IMSC_FE	CASTV(1<<7)	/* FERIS = Framing Interrupt Raw Status */
#define UART0_IMSC_RT	CASTV(1<<6)	/* RTRIS = Receivce Timeout Raw Status */
#define UART0_IMSC_TX	CASTV(1<<5)  /* TXRIS = Transmit Interrupt Raw Status */
#define UART0_IMSC_RX	CASTV(1<<4)	/* RXRIS = Receive Masked Interrupt Raw Status */
				/*    3 = DSRRIS (unsupported) */
				/*    2 = DCDRIS (unsupported) */
#define UART0_IMSC_CTS	CASTV(1<<1)	/* CTSRIS = nUARTCTS Raw Status */
				/*    0 = RIRIS (unsupported) */
#define UART0_RIS	CASTA(UART0_BASE + 0x3C)
/* MIS = Masked Interrupt Status */
/*   10 = OEMIS = Overrun Interrupt Masked Status */
/*    9 = BEMIS = Break Interrupt Masked Status*/
/*    8 = PEMIS = Parity Interrupt Masked Status */
/*    7 = FEMIS = Framing Interrupt Masked Status */
/*    6 = RTMIS = Receivce Timeout Masked Status */
/*    5 = TXMIS = Transmit Interrupt Masked Status */
/*    4 = RXMIS = Receive Masked Interrupt Masked Status */
/*    3 = DSRMIS (unsupported) */
/*    2 = DCDMIS (unsupported) */
/*    1 = CTSMIS = nUARTCTS Masked Status */
/*    0 = RIMIS (unsupported) */
#define UART0_MIS	CASTA(UART0_BASE + 0x40)
/* ICR = Interrupt Clear Register */
/*   10 = OEIC = Overrun Interrupt Clear */
/*    9 = BEIC = Break Interrupt Clear */
/*    8 = PEIC = Parity Interrupt Clear */
/*    7 = FEIC = Framing Interrupt Clear */
/*    6 = RTIC = Receivce Timeout Interrupt Clear */
/*    5 = TXIC = Transmit Interrupt Clear */
/*    4 = RXIC = Receive Masked Interrupt Status */
/*    3 = DSRMIC (unsupported) */
/*    2 = DCDMIC (unsupported) */
/*    1 = CTSMIC = nUARTCTS status */
/*    0 = RIMIC (unsupported) */
#define UART0_ICR	CASTA(UART0_BASE + 0x44)
/* DMACR = DMA Control Register */
/*  This is disabled on the BCM2835 */
#define UART0_DMACR	CASTA(UART0_BASE + 0x48)
/* ITCR = Test Control Register */
#define UART0_ITCR	CASTA(UART0_BASE + 0x80)
/* ITIP = Test Control Register */
#define UART0_ITIP	CASTA(UART0_BASE + 0x84)
/* ITOP = Test Control Register */
#define UART0_ITOP	CASTA(UART0_BASE + 0x88)
/* TDR = Test Data Register */
#define UART0_TDR	CASTA(UART0_BASE + 0x8C)


/***********/
/* MAILBOX */
/***********/
#define MAIL_BASE	CASTA(IO_BASE + 0xB880)
#define MAIL_READ	CASTA(MAIL_BASE + 0x0)
#define MAIL_POLL	CASTA(MAIL_BASE + 0x10)
#define MAIL_STAT	CASTA(MAIL_BASE + 0x18)
#define MAIL_WRITE	CASTA(MAIL_BASE + 0x20)

/*******/
/* DMA */
/*******/
#define DMA0_BASE	CASTA(IO_BASE + 0x7000)

#define DMA_CH0     CASTA(DMA0_BASE + 0x000)
#define DMA_CH1     CASTA(DMA0_BASE + 0x100)
#define DMA_CH2     CASTA(DMA0_BASE + 0x200)
#define DMA_CH3     CASTA(DMA0_BASE + 0x300)
#define DMA_CH4     CASTA(DMA0_BASE + 0x400)
#define DMA_CH5     CASTA(DMA0_BASE + 0x500)
#define DMA_CH6     CASTA(DMA0_BASE + 0x600)
#define DMA_CH7     CASTA(DMA0_BASE + 0x700)
#define DMA_CH8     CASTA(DMA0_BASE + 0x800)
#define DMA_CH9     CASTA(DMA0_BASE + 0x900)
#define DMA_CH10    CASTA(DMA0_BASE + 0xa00)
#define DMA_CH11    CASTA(DMA0_BASE + 0xb00)
#define DMA_CH12    CASTA(DMA0_BASE + 0xc00)
#define DMA_CH13    CASTA(DMA0_BASE + 0xd00)
#define DMA_CH14    CASTA(DMA0_BASE + 0xe00)

#define DMA_CS_OFF              0x000
#define DMA_CS_RESET            CASTV(1 << 31)
#define DMA_CS_ABORT            CASTV(1 << 30)
#define DMA_CS_DISDEBUG         CASTV(1 << 29)
#define DMA_CS_WAIT_WRITE       CASTV(1 << 28)
#define DMA_CS_PANIC_PRIORITY   CASTV(1 << 20)
#define DMA_CS_PRIORITY         CASTV(1 << 16)
#define DMA_CS_ERROR            CASTV(1 << 8)
#define DMA_CS_WAITING_WRITE    CASTV(1 << 6)
#define DMA_CS_DREQ_PAUSE       CASTV(1 << 5)
#define DMA_CS_PAUSED           CASTV(1 << 4)
#define DMA_CS_DREQ             CASTV(1 << 3)
#define DMA_CS_INT              CASTV(1 << 2)
#define DMA_CS_END              CASTV(1 << 1)
#define DMA_CS_ACTIVE           CASTV(1 << 0)

#define DMA_CONBLK_ADDR_OFF     0x004

#define DMA_TI_OFF              0x008
#define DMA_TI_NO_WIDE_BURSTS   CASTV(1 << 26)
#define DMA_TI_WAITS            CASTV(1 << 21)
#define DMA_TI_PERMAP           CASTV(1 << 16)
#define DMA_TI_BURST_LENGTH     CASTV(1 << 12)
#define DMA_TI_SRC_IGNORE       CASTV(1 << 11)
#define DMA_TI_SRC_DREQ         CASTV(1 << 10)
#define DMA_TI_SRC_WIDTH        CASTV(1 << 9)
#define DMA_TI_SRC_INC          CASTV(1 << 8)
#define DMA_TI_DEST_IGNORE      CASTV(1 << 7)
#define DMA_TI_DEST_DREQ        CASTV(1 << 6)
#define DMA_TI_DEST_WIDTH       CASTV(1 << 5)
#define DMA_TI_DEST_INC         CASTV(1 << 4)
#define DMA_TI_WAIT_RESP        CASTV(1 << 3)
#define DMA_TI_TDMODE           CASTV(1 << 1)
#define DMA_TI_INTEN            CASTV(1 << 0)

#define DMA_SOURCE_ADDR_OFF     0x00c
#define DMA_DEST_ADDR_OFF       0x010
#define DMA_TXFR_LEN_OFF        0x014
#define DMA_STRIDE_OFF          0x018
#define DMA_NEXT_CB_OFF         0x01c
#define DMA_DEBUG_OFF           0x020

#define DMA_INT_STATUS          (DMA0_BASE + 0xfe0)
#define DMA_ENABLE              (DMA0_BASE + 0xff0)

#define DMA_DREQ_GENERIC        (0)
#define DMA_DREQ_DSIO_PWM1      (1)
#define DMA_DREQ_PCM_TX         (2)
#define DMA_DREQ_PCM_RX         (3)
#define DMA_DREQ_SMI            (4)
#define DMA_DREQ_PWM0           (5)
#define DMA_DREQ_SPI0_RX        (6)
#define DMA_DREQ_SPI0_TX        (7)
#define DMA_DREQ_BSC_SPI_TX     (8)
#define DMA_DREQ_BSC_SPI_RX     (9)
#define DMA_DREQ_HDMI0          (10)
#define DMA_DREQ_EMMC           (11)
#define DMA_DREQ_UART0_TX       (12)
#define DMA_DREQ_SD_HOST        (13)
#define DMA_DREQ_UART0_RX       (14)
#define DMA_DREQ_DSI1           (15)
#define DMA_DREQ_SPI1_TX        (16)
#define DMA_DREQ_HDMI1          (17)
#define DMA_DREQ_SPI1_RX        (18)
#define DMA_DREQ_UART3_TX_SPI4_TX   (19)
#define DMA_DREQ_UART3_RX_SPI4_RX   (20)
#define DMA_DREQ_UART5_TX_SPI5_TX   (21)
#define DMA_DREQ_UART5_RX_SPI5_RX   (22)
#define DMA_DREQ_SPI6_TX            (23)
#define DMA_DREQ_SCALER_FIFO0       (24)
#define DMA_DREQ_SCALER_FIFO1       (25)
#define DMA_DREQ_SCALER_FIFO2       (26)
#define DMA_DREQ_SPI6_RX            (27)
#define DMA_DREQ_UART2_TX           (28)
#define DMA_DREQ_UART2_RX           (29)
#define DMA_DREQ_UART4_TX           (30)
#define DMA_DREQ_UART4_RX           (31)

#endif
