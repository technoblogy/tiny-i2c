# TinyI2C Library

**TinyI2C** is a set of minimal I2C routines that allow just about any Microchip/Atmel AVR processor to connect to I2C peripherals.

For more information and examples see [Tiny I2C Routines for all AVR Microcontrollers](http://www.technoblogy.com/show?3UF0).

The main difference between these routines and the standard Arduino Wire library is that these don't use buffers, so have much smaller memory requirements and don't impose a limit on transmissions.

Version 2.0.1 increases the number of bytes you can specify in a single transfer.

## Compatibility
These I2C routines are designed to provide master I2C functionality for all Microchip/Atmel AVR processors. Over the years different generations of AVR chips have featured three different, incompatible peripherals to handle I2C:

#### Universal Serial interface (USI) peripheral

The USI provides master I2C support to ATtiny processors with a USI peripheral, namely:

* ATtiny25/45/85 and ATtiny24/44/84
* ATtiny261/461/861
* ATtiny87/167
* ATtiny2313/4313
* ATtiny1634


#### 2-Wire Serial Interface (TWI) peripheral

This provides full master I2C support, and is featured in:

* Most of the original ATmega processors, such as the ATmega328P used in the Arduino Uno, ATmega2560 used in the Arduino Mega 2560, and the ATmega1284P.
* Two unusual ATtiny processors that provide a TWI peripheral, the ATtiny48 and 88.

#### Two-Wire Interface (TWI) peripheral

A new version of the TWI peripheral is featured in:

* The latest ATtiny 0-series, 1-series, and 2-series processors, such as the ATtiny414.
* The 0-series ATmega chips, such as the ATmega4809.
* The AVR DA and DB family, such as the AVR128DA48.

These universal Tiny I2C routines provide master I2C support to all three generations of AVR processors.

## Differences from Arduino Wire

I've named these routines TinyI2C for two reasons: to distinguish them from the existing Wire libraries, such as the ones included in the Arduino and Spence Konde's cores, and to emphasise that these routines don't follow the Arduino Wire library naming conventions.

In addition, these routines differ from the Arduino Wire library routines in the following ways:

#### Low memory requirements

These routines don't use buffers, reducing their RAM requirements to a couple of bytes. The standard 0-series ATmega Wire library uses 128-byte send and receive buffers, and the 0-series and 1-series ATtiny Wire libraries use 32-byte or 16-byte buffers, which on the smaller chips is a significant part of the available RAM. As far as I can see there's no need for buffering as the I2C protocol incorporates handshaking, using the ACK/NACK pulses.

#### Unlimited transmission length

These routines don't impose any limit on the length of transmissions. The standard Wire libraries limit the length of any transmission to the size of the buffer. This isn't a problem with many I2C applications, such as reading the temperature from a sensor, but it is a problem with applications such as driving an I2C OLED display, which requires you to send 1024 bytes to update the whole display.

#### Flexible read

These routines allow you to specify in advance how many bytes you want to read from an I2C peripheral, or you can leave this open-ended and mark the last byte read. This is an advantage when you don't know in advance how many bytes you are going to want to read.

## Polling

For simplicity these routines use polling rather than interrupts, so they won't interfere with other processes using interrupts.

## Description

Install TinyI2C into your libraries folder in your Arduino folder and include this at the top of your program:

    #include <TinyI2CMaster.h>

Here's a description of the minimal TinyI2C routines:

#### TinyI2C.init()

Initialises TinyI2C. This should be called in setup().

#### TinyI2C.start(address, type)

Starts a transaction with the slave device at the specified address, and specifies if the transaction is going to be a read or a write. It returns true if the start was successful or false if there was an error.

The **type** parameter can have the following values:

* 0: Write to the device.
* 1 to 2147483647: Read from the device. The number specifies how many bytes you are going to read.
* -1: Read an unspecified number of bytes from the device.

If **type** is specified as -1 you must identify the last byte read by calling **TinyI2C.readlast()** rather than  **TinyI2C.read()**.

#### TinyI2C.write(data)

Writes a byte of data to a slave device. It returns true if the write was successful or false if there was an error.

#### TinyI2C.read()

Reads a byte from a slave device and returns it.

#### TinyI2C.readLast()

Reads a byte from a slave device and tells the slave to stop sending.

You only need to use **TinyI2C.readlast()** if you called **TinyI2C.start()** or **TinyI2C.restart()** with **type** set to -1.

#### TinyI2C.restart(address, type);

Does a restart. The **type** parameter is the same as for  **TinyI2C.start()**.

#### TinyI2C.stop()

Ends the transaction; there's no return value.

Every **TinyI2C.start()** should have a matching  **TinyI2C.stop()**.

## Using the TinyI2C library

#### Pullup resistors

You must have pullup resistors on the SCL and SDA lines for I2C to work reliably; recommended values are 4.7kΩ or 10kΩ. On platforms where this is possible the TinyI2C routines turn on the internal pullups on the SCL and SDA lines as this can't do any harm, but you shouldn't rely on these.

#### Writing to an I2C device

Writing to an I2C device is straightforward: for example, to write one byte:

````
TinyI2C.start(Address, 0);
TinyI2C.write(byte);
TinyI2C.stop();
````
#### Reading from an I2C device

The TinyI2C routines allow you to identify the last byte read from an I2C device in either of two ways:

You can specify the total number of bytes you are going to read, as the second parameter of **TinyI2C.start()**. With this approach **TinyI2C.read()** will automatically terminate the last call with a NAK:

````
TinyI2C.start(Address, 2);
int mins = TinyI2C.read();
int hrs = TinyI2C.read();
TinyI2C.stop();
````

Alternatively you can just specify the second parameter of **TinyI2C.start()** as -1, and explicitly identify the last **TinyI2C.read** command by calling **TinyI2C.readlast()**:

````
TinyI2C.start(Address, -1);
int mins = TinyI2C.read();
int hrs = TinyI2C.readLast();
TinyI2C.stop();
````

#### Writing and reading

Many I2C devices require you to write one or more bytes before reading, to specify the register you want to read from; the read should be introduced with a **TinyI2C.restart()** call; for example:

````
TinyI2C.start(Address, 0);
TinyI2C.write(1);
TinyI2C.restart(Address, 2);
int mins = TinyI2C.read();
int hrs = TinyI2C.read();
TinyI2C.stop();
````
