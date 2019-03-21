/* Digital Clock using TinyI2C Library

   David Johnson-Davies - www.technoblogy.com - 6th June 2018
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <TinyI2CMaster.h>

// Digital clock **********************************************

const int RTCaddress = 0x68;
const int DisplayAddress = 0x70;
int Colon = 2;

char Segment[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void SetClock (int hr, int min) {
  TinyI2C.start(RTCaddress, 0);
  TinyI2C.write(0);
  TinyI2C.write(0);
  TinyI2C.write(min);
  TinyI2C.write(hr);
  TinyI2C.stop();
}

void InitDisplay () {
  TinyI2C.start(DisplayAddress, 0);
  TinyI2C.write(0x21);
  TinyI2C.restart(DisplayAddress, 0);
  TinyI2C.write(0x81);
  TinyI2C.restart(DisplayAddress, 0); 
  TinyI2C.write(0xe1);
  TinyI2C.stop();
}

void WriteWord (uint8_t b) {
  TinyI2C.write(b);
  TinyI2C.write(0);
} 
  
// Setup **********************************************

void setup() {
  TinyI2C.init();
  InitDisplay();
  SetClock(0x12, 0x34);      // Set the time to 12:34
}

void loop () {
  // Read the time from the RTC
  TinyI2C.start(RTCaddress, 0);
  TinyI2C.write(1);
  TinyI2C.restart(RTCaddress, 2);
  int mins = TinyI2C.read();
  int hrs = TinyI2C.read();
  TinyI2C.stop();

  // Write the time to the display
  TinyI2C.start(DisplayAddress, 0);
  TinyI2C.write(0);
  WriteWord(Segment[hrs / 16]);
  WriteWord(Segment[hrs % 16]);
  WriteWord(Colon);
  WriteWord(Segment[mins / 16]);
  WriteWord(Segment[mins % 16]);
  TinyI2C.stop();
  // Flash the colon
  Colon = 2 - Colon;
  delay(1000);
}


