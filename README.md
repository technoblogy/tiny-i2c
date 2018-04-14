# Arduino TinyI2C Library
David Johnson-Davies 14th April 2018  

## Description
*TinyI2C* is a set of minimal I2C Master routines for ATtiny processors with a hardware USI.

The main difference between these routines and most other Tiny Wire libraries is that these don't use buffers, so have minimal memory requirements, and don't impose a 32-byte limit on transmissions.

## Introduction

I've named these routines TinyI2C for two reasons: to distinguish them from the existing TinyWire libraries, such as the one included in Spence Konde's ATTiny Core, and to emphasise that these routines don't follow the Arduino Wire library naming conventions.

In addition, these routines differ from the Tiny Wire library routines in the following ways:

### Low memory requirements

These routines don't use buffers, reducing their RAM requirements to a couple of bytes. The standard Wire libraries use 32-byte send and receive buffers requiring at least 32 bytes, which isn't such a problem on the ATmega chips, but on an ATtiny85 this is a significant part of the available RAM.

I've always been baffled about why the standard wire libraries use 32-byte send and receive buffers, and I haven't been able to find an answer to this on the web. As far as I can see there's no need for buffering as the I2C protocol incorporates handshaking, using the ACK/NACK pulses.

### Unlimited transmission length

These routines don't impose any limit on the length of transmissions. The standard Wire libraries limit the length of any transmission to 32 bytes. This isn't a problem with many I2C applications, such as reading the temperature from a sensor, but it is a problem with applications such as driving an I2C OLED display, which requires you to send 1024 bytes to update the whole display.

### Flexible read

These routines allow you to specify in advance how many bytes you want to read from an I2C peripheral, or you can leave this open-ended and mark the last byte read. This is an advantage when you don't know in advance how many bytes you are going to want to read.

## Compatibility

Although so far I've only tested these routines on a couple of ATtiny chips, they should support all ATtiny chips with the USI peripheral, namely:

* ATtiny 25/45/85
* ATtiny 24/44/84
* ATtiny 261/461/861
* ATtiny 2313/4313
* ATtiny 1634

These routines are based on the code described by Atmel Application Note AVR310 "Using the USI module as a TWI Master".

## Description

Here's a description of the Minimal Tiny I2C routines:

### TinyI2C.start(address, type)

Starts a transaction with the slave device with the specified address, and specifies if the transaction is going to be a read or a write. It returns a true/false value to say whether the start was successful.

The **type** parameter can have the following values:

* 0: Write to the device.
* 1 to 32767: Read from the device. The number specifies how many reads you are going to do.
* -1: Read an unspecified number of bytes from the device.

If **type** is specified as -1 you must identify the last read by calling **TinyI2C.readLast()** rather than **TinyI2C.read()**.

### TinyI2C.write(data)

Writes a byte of data to a slave device. It returns a true/false value to say whether the write was successful.

### TinyI2C.read()

Returns the result of reading from a slave device.

### TinyI2C.readLast()

Returns the result of reading from a slave device and tells the slave to stop sending.

### TinyI2C.restart(address, type);

Does a restart. The **type** parameter is the same as for **TinyI2C.start()**.

### TinyI2C.stop()

Ends the transaction.

## Examples

### Writing to a slave

Writing to a slave is straightforward: for example, to write one byte:

    TinyI2C.start(Address, 0);
    TinyI2C.write(byte);
    TinyI2C.stop();

### Reading from a slave

The TinyI2C routines allow you to identify the last byte read from a slave in either of two ways:

You can specify the total number of bytes you are going to read, as the second parameter of **TinyI2C.start()**. With this approach **TinyI2C.read()** will automatically terminate the last call with a NAK:

    TinyI2C.start(Address, 2);
    int mins = TinyI2C.read();
    int hrs = TinyI2C.read();
    TinyI2C.stop();

Alternatively you can just specify the second parameter of **TinyI2C.start()** as -1, and explicitly identify the last **TinyI2C.read** command by calling **TinyI2C.readLast()**:

    TinyI2C.start(Address, -1);
    int mins = TinyI2C.read();
    int hrs = TinyI2C.readLast();
    TinyI2C.stop();

### Writing and reading

Many I2C devices require you to write one or more bytes before reading, to specify the register you want to read from; the read should be introduced with an **vrestart()** call; for example:

    TinyI2C.start(Address, 0);
    TinyI2C.write(1);
    TinyI2C.restart(Address, 2);
    int mins = TinyI2C.read();
    int hrs = TinyI2C.read();
    TinyI2C.stop();
