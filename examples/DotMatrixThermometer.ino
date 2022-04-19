/* Dot-Matrix Thermometer using TinyI2C library - see http://www.technoblogy.com/show?3UF0

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

// Character set 0 to 9
const uint32_t PROGMEM Digits [8] = {
0b010010111010111001010010010010,
0b101101100101001001101101011101,
0b101101100001001101100100010101,
0b110010010011011111010010010101,
0b100101001101100100100001010101,
0b101101001101101100101001010101,
0b010010001010010100010111111010 };

// Display a digit at specified x position on a display
void PlotDigit (int address, uint8_t x, uint8_t n) {
  for (int y=0; y<7; y++) {
    TinyI2C.start(address, 0);
    TinyI2C.write(y * 2);
    TinyI2C.restart(address, 1);
    uint8_t row = TinyI2C.read();
    TinyI2C.restart(address, 0);
    TinyI2C.write(y * 2);
    uint8_t b = (pgm_read_dword(&Digits[y])>>(3*n) & 7);
    TinyI2C.write((row & ~((7<<x | 7<<(8+x))>>1)) | (b<<x | b<<(8+x))>>1);
    TinyI2C.stop();
  }
}

// Displays degree sign and decimal point
void Symbols (int address) {
 TinyI2C.start(address, 0);
 TinyI2C.write(0);
 TinyI2C.write(0x60);
 TinyI2C.write(0);
 TinyI2C.write(0x60);
 TinyI2C.restart(address, 0);
 TinyI2C.write(6*2);
 TinyI2C.write(0x80);
 TinyI2C.stop();
}

// Thermometer **********************************************

const int ThermometerAddress = 0x37;

// Returns temperature in eighths of a degree
int PCT2075Temp (int address) {
  TinyI2C.start(address, 0);
  TinyI2C.write(0);
  TinyI2C.restart(address, 2);
  uint8_t hi = TinyI2C.read();
  uint8_t lo = TinyI2C.read();
  TinyI2C.stop();
  int temp = hi<<3 | lo>>5;
  return (temp & 0x03FF) - (temp & 0x0400);
}
      
// Setup **********************************************

void setup () {
  TinyI2C.init();
  InitDisplay(DisplayLeft);
  InitDisplay(DisplayRight);
  ClearDisplay(DisplayLeft);
  ClearDisplay(DisplayRight);
  Symbols(DisplayRight);
}

void loop () {
  int temp = PCT2075Temp(ThermometerAddress) - 8;
  PlotDigit(DisplayLeft,0,temp/80);
  PlotDigit(DisplayLeft,4,temp/8 % 10);
  PlotDigit(DisplayRight,2,(temp%8 * 10 + 4)/8);
  delay(1000);
}
