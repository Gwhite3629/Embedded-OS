#ifndef _GPIO_H_
#define _GPIO_H_

#define CAST(X) (volatile unsigned int *(X))

#define GPIO_BASE   IO_BASE + 0x00200000

#define GPFSEL0     CAST(GPIO_BASE + 0x00)
#define GPFSEL1     CAST(GPIO_BASE + 0x04)
#define GPFSEL2     CAST(GPIO_BASE + 0x08)
#define GPFSEL3     CAST(GPIO_BASE + 0x0C)
#define GPFSEL4     CAST(GPIO_BASE + 0x10)
#define GPFSEL5     CAST(GPIO_BASE + 0x14)
#define GPSET0      CAST(GPIO_BASE + 0x1C)
#define GPSET1      CAST(GPIO_BASE + 0x20)
#define GPCLR0      CAST(GPIO_BASE + 0x28)
#define GPLEV0      CAST(GPIO_BASE + 0x34)
#define GPLEV1      CAST(GPIO_BASE + 0x38)
#define GPEDS0      CAST(GPIO_BASE + 0x40)
#define GPEDS1      CAST(GPIO_BASE + 0x44)
#define GPHEN0      CAST(GPIO_BASE + 0x64)
#define GPHEN1      CAST(GPIO_BASE + 0x68)
#define GPPUD       CAST(GPIO_BASE + 0x94)
#define GPPUDCLK0   CAST(GPIO_BASE + 0x98)
#define GPPUDCLK1   CAST(GPIO_BASE + 0x9C)

#endif // _GPIO_H_
