//#include <stddef.h>
#include <drivers/io.h>
#include <stdlib.h>

void uart_init(void) {

	unsigned int old;

	//Disable UART
	chip_write(0x0, UART0_CR);

	//Setup GPIO pins 14 and 15

	// Set GPIO14 and GPIO15 to be pl011 TX, so ALT0
	// ALT0 is binary 100 (0x4)
	old=chip_read(GPIO_GPFSEL1);
	old &= ~(0x7 << 12);
	old |= (4<<12);

	old &= ~(0x7 << 15);
	old |= (4<<15);
	chip_write(old, GPIO_GPFSEL1);

	// Disable the pull up/down on pins 14 and 15
	// See the Peripheral Manual for more info
	// Configure to disable pull up/down and delay for 150 cycles
	chip_write(GPIO_GPPUD_DISABLE, GPIO_GPPUD);
	delay(10);

	// Pass the disable clock to GPIO pins 14 and 15 and delay
	chip_write((1 << 14) | (1 << 15), GPIO_GPPUDCLK0);
	delay(10);

	// Write 0 to GPPUDCLK0 to make it take effect
	chip_write(0x0, GPIO_GPPUDCLK0);

	// Mask all interrupts.
	chip_write(0, UART0_IMSC);

	// Clear pending interrupts.
	chip_write(0x7FF, UART0_ICR);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 48MHz; Baud = 115200.

	// Clock changed from 3MHz to 48MHz on newer firmwares + pi3

	chip_write(26, UART0_IBRD);
	chip_write(3, UART0_FBRD);

	// Set up the Line Control Register
	// Enable FIFO
	// Set length to 8 bit
	// Defaults for other bit are No parity, 1 stop bit
	chip_write(UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT, UART0_LCRH);

	// Enable UART0, receive, and transmit
	chip_write(UART0_CR_UARTEN |
				UART0_CR_TXE |
				UART0_CR_RXE,
				UART0_CR);
}

void uart_putc(unsigned char byte) {

	// Check Flags Register 
	// And wait until FIFO not full 
	while ( chip_read(UART0_FR) & UART0_FR_TXFF ) {
	}

	// Write our data byte out to the data register 
	chip_write(byte, UART0_DR);
}

unsigned char uart_getc(void) {

	// Check Flags Register 
	// Wait until Receive FIFO is not empty 
	while ( chip_read(UART0_FR) & UART0_FR_RXFE ) {
	}

	// Read and return the received data 
	// Note we are ignoring the top 4 error bits 

	return chip_read(UART0_DR);
}

int32_t uart_getc_noblock(void) {

	// Check Flags Register 

	// Return 0 if Receive FIFO is empty 
	if ( chip_read(UART0_FR) & UART0_FR_RXFE ) {
		return 0;
	}

	// Read and return the received data 
	// Note we are ignoring the top 4 error bits 

	return (chip_read(UART0_DR))&0xff;
}


// write a series of bytes to the serial port 
uint32_t uart_write(const unsigned char* buffer, size_t size) {

	size_t i;

	for ( i = 0; i < size; i++ ) {
		uart_putc(buffer[i]);
		if (buffer[i]=='\n') uart_putc('\r');
	}
	return i;
}