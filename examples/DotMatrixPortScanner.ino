/* Dot-Matrix I2C Port Scanner using TinyI2C library - see http://www.technoblogy.com/show?3UF0

   David Johnson-Davies - www.technoblogy.com - 19th April 2022   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <TinyI2CMaster.h>

// Dot matrix display **********************************************

const int DisplayLeft  = 0x70;
const int DisplayRight = 0x71;
const int Brightness = 1;

void InitDisplay (int address) {
  TinyI2C.start(address, 0);
  TinyI2C.write(0x21);
  TinyI2C.restart(address, 0);
  TinyI2C.write(0x81);
  TinyI2C.restart(address, 0); 
  TinyI2C.write(0xe0 + Brightness);
  TinyI2C.stop();
}

void ClearDisplay (int address) {
  TinyI2C.start(address, 0);
  for (int i=0; i<17; i++) TinyI2C.write(0);
  TinyI2C.stop();
}

// Plot a pixel at x,y
void Plot (int x, int y) {
  uint8_t address;
  if (x > 7) address = DisplayRight; else address = DisplayLeft;
  TinyI2C.start(address, 0);
  TinyI2C.write(y * 2);
  TinyI2C.restart(address, 1);
  uint8_t row = TinyI2C.read();
  TinyI2C.restart(address, 0);
  TinyI2C.write(y * 2);
  TinyI2C.write(row | 1<<((x + 7) & 7));
  TinyI2C.stop();
}

// Setup **********************************************

void setup() {
  TinyI2C.init();
  InitDisplay(DisplayLeft);
  InitDisplay(DisplayRight);
  ClearDisplay(DisplayLeft);
  ClearDisplay(DisplayRight);
}

void loop () {
  for (int p=0; p<128; p++) {
    if (!TinyI2C.start(p, 0)) Plot(p&15, p>>4);
    delay(50);
  }
  for(;;);
}
