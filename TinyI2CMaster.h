/* Tiny I2C

   David Johnson-Davies - www.technoblogy.com - 14th April 2018
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#ifndef TinyI2CMaster_h
#define TinyI2CMaster_h

#include <stdint.h>
#include <arduino.h>
#include <avr/io.h>
#include <util/delay.h>

// Defines
#define TWI_FAST_MODE

#ifdef TWI_FAST_MODE                 // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// Constants
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const unsigned char USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const unsigned char USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

class TinyI2CMaster {

    public:
	TinyI2CMaster();
	void init(void);
	uint8_t read(void);
	uint8_t readLast(void);
	bool write(uint8_t data);
	bool start(uint8_t address, int readcount);
	bool restart(uint8_t address, int readcount);
	void stop(void);

    private:
	int I2Ccount;
	uint8_t transfer(uint8_t data);
};

extern TinyI2CMaster TinyI2C;

#endif

