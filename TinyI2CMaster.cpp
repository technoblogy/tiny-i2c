/* TinyI2C v2.0.1

   David Johnson-Davies - www.technoblogy.com - 5th June 2022
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include "TinyI2CMaster.h"

TinyI2CMaster::TinyI2CMaster() {
}

#if defined(USIDR)

/* *********************************************************************************************************************

   Minimal Tiny I2C Routines for original ATtiny chips that support I2C using a USI peripheral, such as ATtiny85.

********************************************************************************************************************* */

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
unsigned char const USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
unsigned char const USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

uint8_t TinyI2CMaster::transfer (uint8_t data) {
  USISR = data;                                                   // Set USISR according to data.
                                                                  // Prepare clocking.
  data = 0<<USISIE | 0<<USIOIE |                                  // Interrupts disabled
         1<<USIWM1 | 0<<USIWM0 |                                  // Set USI in Two-wire mode.
         1<<USICS1 | 0<<USICS0 | 1<<USICLK |                      // Software clock strobe as source.
         1<<USITC;                                                // Toggle Clock Port.
  do {
    DELAY_T2TWI;
    USICR = data;                                                 // Generate positive SCL edge.
    while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                       // Wait for SCL to go high.
    DELAY_T4TWI;
    USICR = data;                                                 // Generate negative SCL edge.
  } while (!(USISR & 1<<USIOIF));                                 // Check for transfer complete.

  DELAY_T2TWI;
  data = USIDR;                                                   // Read out data.
  USIDR = 0xFF;                                                   // Release SDA.
  DDR_USI |= (1<<PIN_USI_SDA);                                    // Enable SDA as output.

  return data;                                                    // Return the data from the USIDR
}

void TinyI2CMaster::init () {
  PORT_USI |= 1<<PIN_USI_SDA;                                     // Enable pullup on SDA.
  PORT_USI_CL |= 1<<PIN_USI_SCL;                                  // Enable pullup on SCL.

  DDR_USI_CL |= 1<<PIN_USI_SCL;                                   // Enable SCL as output.
  DDR_USI |= 1<<PIN_USI_SDA;                                      // Enable SDA as output.

  USIDR = 0xFF;                                                   // Preload data register with data.
  USICR = 0<<USISIE | 0<<USIOIE |                                 // Disable Interrupts.
          1<<USIWM1 | 0<<USIWM0 |                                 // Set USI in Two-wire mode.
          1<<USICS1 | 0<<USICS0 | 1<<USICLK |                     // Software stobe as counter clock source
          0<<USITC;
  USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | // Clear flags,
          0x0<<USICNT0;                                           // and reset counter.
}

uint8_t TinyI2CMaster::read (void) {
  if ((I2Ccount != 0) && (I2Ccount != -1)) I2Ccount--;
  
  /* Read a byte */
  DDR_USI &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  uint8_t data = TinyI2CMaster::transfer(USISR_8bit);

  /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
  if (I2Ccount == 0) USIDR = 0xFF; else USIDR = 0x00;
  TinyI2CMaster::transfer(USISR_1bit);                            // Generate ACK/NACK.

  return data;                                                    // Read successfully completed
}

uint8_t TinyI2CMaster::readLast (void) {
  I2Ccount = 0;
  return TinyI2CMaster::read();
}

bool TinyI2CMaster::write (uint8_t data) {
  /* Write a byte */
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);                               // Pull SCL LOW.
  USIDR = data;                                                   // Setup data.
  TinyI2CMaster::transfer(USISR_8bit);                            // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDR_USI &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  if (TinyI2CMaster::transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

  return true;                                                    // Write successfully completed
}

// Start transmission by sending address
bool TinyI2CMaster::start (uint8_t address, int32_t readcount) {
  if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
  uint8_t addressRW = address<<1 | readcount;

  /* Release SCL to ensure that (repeated) Start can be performed */
  PORT_USI_CL |= 1<<PIN_USI_SCL;                                  // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                         // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
  DELAY_T4TWI;
#else
  DELAY_T2TWI;
#endif

  /* Generate Start Condition */
  PORT_USI &= ~(1<<PIN_USI_SDA);                                  // Force SDA LOW.
  DELAY_T4TWI;
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);                               // Pull SCL LOW.
  PORT_USI |= 1<<PIN_USI_SDA;                                     // Release SDA.

  if (!(USISR & 1<<USISIF)) return false;

  /*Write address */
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);                               // Pull SCL LOW.
  USIDR = addressRW;                                              // Setup data.
  TinyI2CMaster::transfer(USISR_8bit);                            // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDR_USI &= ~(1<<PIN_USI_SDA);                                   // Enable SDA as input.
  if (TinyI2CMaster::transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

  return true;                                                    // Start successfully completed
}

bool TinyI2CMaster::restart(uint8_t address, int32_t readcount) {
  return TinyI2CMaster::start(address, readcount);
}

void TinyI2CMaster::stop (void) {
  PORT_USI &= ~(1<<PIN_USI_SDA);                                  // Pull SDA low.
  PORT_USI_CL |= 1<<PIN_USI_SCL;                                  // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));                         // Wait for SCL to go high.
  DELAY_T4TWI;
  PORT_USI |= 1<<PIN_USI_SDA;                                     // Release SDA.
  DELAY_T2TWI;
}

#elif defined(TWDR)

/* *********************************************************************************************************************

   Minimal Tiny I2C Routines for most of the original ATmega processors, such as the ATmega328P used in
   the Arduino Uno, ATmega32U4, ATmega2560 used in the Arduino Mega 2560, and the ATmega1284P,
   plus a few unusual ATtiny processors that provide a TWI peripheral: ATtiny48/88.

********************************************************************************************************************* */


// 400kHz clock
uint32_t const F_TWI = 400000L;                                   // Hardware I2C clock in Hz

// Choose for 1MHz clock
//uint32_t const F_TWI = 1000000L;                                // Hardware I2C clock in Hz

uint8_t const TWSR_MTX_DATA_ACK = 0x28;
uint8_t const TWSR_MTX_ADR_ACK = 0x18;
uint8_t const TWSR_MRX_ADR_ACK = 0x40;
uint8_t const TWSR_START = 0x08;
uint8_t const TWSR_REP_START = 0x10;
uint8_t const I2C_READ = 1;
uint8_t const I2C_WRITE = 0;

void TinyI2CMaster::init () {
  digitalWrite(SDA, HIGH);                                        // Pullups on
  digitalWrite(SCL, HIGH);
  TWSR = 0;                                                       // No prescaler
  TWBR = (F_CPU/F_TWI - 16)/2;                                    // Set bit rate factor
}

uint8_t TinyI2CMaster::read (void) {
  if (I2Ccount != 0) I2Ccount--;
  TWCR = 1<<TWINT | 1<<TWEN | ((I2Ccount == 0) ? 0 : (1<<TWEA));
  while (!(TWCR & 1<<TWINT));
  return TWDR;
}

uint8_t TinyI2CMaster::readLast (void) {
  I2Ccount = 0;
  return TinyI2CMaster::read();
}

bool TinyI2CMaster::write (uint8_t data) {
  TWDR = data;
  TWCR = 1<<TWINT | 1 << TWEN;
  while (!(TWCR & 1<<TWINT));
  return (TWSR & 0xF8) == TWSR_MTX_DATA_ACK;
}

// Start transmission by sending address
bool TinyI2CMaster::start (uint8_t address, int32_t readcount) {
  bool read;
  if (readcount == 0) read = 0;                                   // Write
  else { I2Ccount = readcount; read = 1; }                        // Read
  uint8_t addressRW = address<<1 | read;
  TWCR = 1<<TWINT | 1<<TWSTA | 1<<TWEN;                           // Send START condition
  while (!(TWCR & 1<<TWINT));
  if ((TWSR & 0xF8) != TWSR_START && (TWSR & 0xF8) != TWSR_REP_START) return false;
  TWDR = addressRW;                                               // Send device address and direction
  TWCR = 1<<TWINT | 1<<TWEN;
  while (!(TWCR & 1<<TWINT));
  if (addressRW & I2C_READ) return (TWSR & 0xF8) == TWSR_MRX_ADR_ACK;
  else return (TWSR & 0xF8) == TWSR_MTX_ADR_ACK;
}

bool TinyI2CMaster::restart(uint8_t address, int32_t readcount) {
  return TinyI2CMaster::start(address, readcount);
}

void TinyI2CMaster::stop (void) {
  TWCR = 1<<TWINT | 1<<TWEN | 1<<TWSTO;
  while (TWCR & 1<<TWSTO); // wait until stop and bus released
}

#elif defined(DXCORE) || defined(MEGATINYCORE) || defined(MEGACOREX)
/* *********************************************************************************************************************

   Minimal Tiny I2C Routines for the new 0-series and 1-series ATtiny and ATmega microcontrollers,
   such as the ATtiny402 or ATmega4809, and the AVR Dx series microcontrollers, such as the AVR128DA48
   and AVR32DB28.

********************************************************************************************************************* */

// 400kHz clock
uint32_t const FREQUENCY = 400000L;                               // Hardware I2C clock in Hz
uint32_t const T_RISE = 300L;                                     // Rise time

// Choose these for 1MHz clock
//uint32_t const FREQUENCY = 1000000L;                            // Hardware I2C clock in Hz
//uint32_t const T_RISE = 120L;                                   // Rise time

void TinyI2CMaster::init () {
#if !defined(DXCORE)
  pinMode(PIN_WIRE_SDA, INPUT_PULLUP);                            // Pullups on unless AVR DA/DB
  pinMode(PIN_WIRE_SCL, INPUT_PULLUP);
#endif
  uint32_t baud = ((F_CPU/FREQUENCY) - (((F_CPU*T_RISE)/1000)/1000)/1000 - 10)/2;
  TWI0.MBAUD = (uint8_t)baud;
  TWI0.MCTRLA = TWI_ENABLE_bm;                                    // Enable as master, no interrupts
  TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

uint8_t TinyI2CMaster::read (void) {
  if (I2Ccount != 0) I2Ccount--;
  while (!(TWI0.MSTATUS & TWI_RIF_bm));                           // Wait for read interrupt flag
  uint8_t data = TWI0.MDATA;
  // Check slave sent ACK?
  if (I2Ccount != 0) TWI0.MCTRLB = TWI_MCMD_RECVTRANS_gc;         // ACK = more bytes to read
  else TWI0.MCTRLB = TWI_ACKACT_NACK_gc;                          // Send NAK
  return data;
}

uint8_t TinyI2CMaster::readLast (void) {
  I2Ccount = 0;
  return TinyI2CMaster::read();
}

bool TinyI2CMaster::write (uint8_t data) {
  TWI0.MCTRLB = TWI_MCMD_RECVTRANS_gc;                            // Prime transaction
  TWI0.MDATA = data;                                              // Send data
  while (!(TWI0.MSTATUS & TWI_WIF_bm));                           // Wait for write to complete
  if (TWI0.MSTATUS & (TWI_ARBLOST_bm|TWI_BUSERR_bm))return false; // Fails if bus error or arblost
  return !(TWI0.MSTATUS & TWI_RXACK_bm);                          // Returns true if slave gave an ACK
}

// Start transmission by sending address
bool TinyI2CMaster::start (uint8_t address, int32_t readcount) {
  bool read;
  if (readcount == 0) read = 0;                                   // Write
  else { I2Ccount = readcount; read = 1; }                        // Read
  TWI0.MADDR = address<<1 | read;                                 // Send START condition
  while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)));            // Wait for write or read interrupt flag
  if (TWI0.MSTATUS & TWI_ARBLOST_bm) {                            // Arbitration lost or bus error
    while (!(TWI0.MSTATUS & TWI_BUSSTATE_IDLE_gc));               // Wait for bus to return to idle state
    return false;
  } else if (TWI0.MSTATUS & TWI_RXACK_bm) {                       // Address not acknowledged by client
    TWI0.MCTRLB |= TWI_MCMD_STOP_gc;                              // Send stop condition
    while (!(TWI0.MSTATUS & TWI_BUSSTATE_IDLE_gc));               // Wait for bus to return to idle state
    return false;
  }
  return true;                                                    // Return true if slave gave an ACK
}

bool TinyI2CMaster::restart(uint8_t address, int32_t readcount) {
  return TinyI2CMaster::start(address, readcount);
}

void TinyI2CMaster::stop (void) {
  TWI0.MCTRLB |= TWI_MCMD_STOP_gc;                                // Send STOP
  while (!(TWI0.MSTATUS & TWI_BUSSTATE_IDLE_gc));                 // Wait for bus to return to idle state
}

#else
#error "Sorry TinyI2C doesn't support this processor"
#endif

// All versions

TinyI2CMaster TinyI2C = TinyI2CMaster();                          // Instantiate a TinyI2C object
