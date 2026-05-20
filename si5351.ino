////////////////////////////////////////////////////////////////////////
// Author: Hans Summers, 2015
// Website: http://www.hanssummers.com
//
// A very very simple Si5351a demonstration
// using the Si5351a module kit http://www.hanssummers.com/synth
// Please also refer to SiLabs AN619 which describes all the registers to use
//----------------------------------------------------------------------
// Modifications: JA2GQP,2017/5/20
//     1)Output is CLK0 and CLK2.
//     2)Arduino and stm32duino Operable.   
////////////////////////////////////////////////////////////////////////

//#include <Wire.h>

#define CLK0_CTRL   16               // Register definitions
#define CLK1_CTRL   17
#define CLK2_CTRL   18
#define MSNA_ADDR   26
#define MSNB_ADDR   34
#define MS0_ADDR    42
#define MS1_ADDR    50
#define MS2_ADDR    58
#define PLL_RESET   177
#define XTAL_LOAD_C 183

#define R_DIV_1      0b00000000     // R-division ratio definitions
#define R_DIV_2      0b00010000
#define R_DIV_4      0b00100000
#define R_DIV_8      0b00110000
#define R_DIV_16     0b01000000
#define R_DIV_32     0b01010000
#define R_DIV_64     0b01100000
#define R_DIV_128    0b01110000

#define Si5351A_ADDR  0x60

#define CLK_SRC_PLL_A 0b00000000
#define CLK_SRC_PLL_B 0b00100000



//------------- momory define ------------

//uint32_t xtalFreq = XTAL_FREQ;    // 2017/9/29

////////////////////////////////////////////////////////////////////////
// I2C write
////////////////////////////////////////////////////////////////////////

void Si5351_write(byte Reg , byte Data){
  Wire.beginTransmission(Si5351A_ADDR);
  Wire.write(Reg);
  Wire.write(Data);
  Wire.endTransmission();
}

////////////////////////////////////////////////////////////////////////
// Set up specified PLL with mult, num and denom
// mult is 15..90
// num is 0..1,048,575 (0xFFFFF)
// denom is 0..1,048,575 (0xFFFFF)
///////////////////////////////////////////////////////////////////////

void setupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom){
  uint32_t P1;                            // PLL config register P1
  uint32_t P2;                            // PLL config register P2
  uint32_t P3;                            // PLL config register P3

  P1 = (uint32_t)(128 * ((float)num / (float)denom));
  P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
  P2 = (uint32_t)(128 * ((float)num / (float)denom));
  P2 = (uint32_t)(128 * num - denom * P2);
  P3 = denom;

  Si5351_write(pll + 0, (P3 & 0x0000FF00) >> 8);
  Si5351_write(pll + 1, (P3 & 0x000000FF));
  Si5351_write(pll + 2, (P1 & 0x00030000) >> 16);
  Si5351_write(pll + 3, (P1 & 0x0000FF00) >> 8);
  Si5351_write(pll + 4, (P1 & 0x000000FF));
  Si5351_write(pll + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  Si5351_write(pll + 6, (P2 & 0x0000FF00) >> 8);
  Si5351_write(pll + 7, (P2 & 0x000000FF));
}

////////////////////////////////////////////////////////////////////////
// Set up MultiSynth with integer divider and R divider
// R divider is the bit value which is OR'ed onto the appropriate 
// register, it is a #define in si5351a.h
////////////////////////////////////////////////////////////////////////

void setupMultisynth(uint8_t synth, uint32_t divider, uint8_t rDiv){
  uint32_t P1;                          // Synth config register P1
  uint32_t P2;                          // Synth config register P2
  uint32_t P3;                          // Synth config register P3

  P1 = 128 * divider - 512;
  P2 = 0;                               // P2 = 0, P3 = 1 forces an integer value for the divider
  P3 = 1;

  Si5351_write(synth + 0, (P3 & 0x0000FF00) >> 8);
  Si5351_write(synth + 1, (P3 & 0x000000FF));
  Si5351_write(synth + 2, ((P1 & 0x00030000) >> 16) | rDiv);
  Si5351_write(synth + 3, (P1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (P1 & 0x000000FF));
  Si5351_write(synth + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (P2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (P2 & 0x000000FF));
}

////////////////////////////////////////////////////////////////////////
// Switches off Si5351a output
// Example: si5351aOutputOff(CLK0_CTRL);
// will switch off output CLK0
////////////////////////////////////////////////////////////////////////

void si5351aOutputOff(uint8_t clk){
  Si5351_write(clk, 0x80);              // Refer to SiLabs AN619 to see 
                                        //bit values - 0x80 turns off the output stage
}

////////////////////////////////////////////////////////////////////////
// Set CLK0 output ON and to the specified frequency
// Frequency is in the range 1MHz to 150MHz
// Example: si5351aSetFrequency(10000000);
// will set output CLK0 to 10MHz
//
// This example sets up PLL A
// and MultiSynth 0
// and produces the output on CLK0
////////////////////////////////////////////////////////////////////////

void si5351aSetFrequency(uint32_t frequency){
  uint32_t pllFreq;
  //uint32_t xtalFreq = XTAL_FREQ;        // 2017/9/29
  uint32_t l;
  float f;
  uint8_t mult;
  uint32_t num;
  uint32_t denom;
  uint32_t divider;

  divider = 900000000 / frequency;        // Calculate the division ratio. 900,000,000 is the maximum internal
                                          // PLL frequency: 900MHz
  if (divider % 2) divider--;             // Ensure an even integer 
                                          //division ratio

  pllFreq = divider * frequency;          // Calculate the pllFrequency: 
                                          //the divider * desired output frequency

  mult = pllFreq / xtalFreq;              // Determine the multiplier to 
                                          //get to the required pllFrequency
  l = pllFreq % xtalFreq;                 // It has three parts:
  f = l;                                  // mult is an integer that must be in the range 15..90
  f *= 1048575;                           // num and denom are the fractional parts, the numerator and denominator
  f /= xtalFreq;                          // each is 20 bits (range 0..1048575)
  num = f;                                // the actual multiplier is mult + num / denom
  denom = 1048575;                        // For simplicity we set the denominator to the maximum 1048575

                                          // Set up PLL A with the calculated  multiplication ratio
  setupPLL(MSNA_ADDR, mult, num, denom);
                                          // Set up MultiSynth divider 0, with the calculated divider.
                                          // The final R division stage can divide by a power of two, from 1..128.
                                          // reprented by constants SI_R_DIV1 to SI_R_DIV128 (see si5351a.h header file)
                                          // If you want to output frequencies below 1MHz, you have to use the
                                          // final R division stage
  setupMultisynth(MS0_ADDR, divider, R_DIV_1);
                                          // Reset the PLL. This causes a glitch in the output. For small changes to
                                          // the parameters, you don't need to reset the PLL, and there is no glitch
  //Si5351_write(PLL_RESET, 0xA0);
                                          // Finally switch on the CLK0 output (0x4F)
                                          // and set the MultiSynth0 input to be PLL A 
  Si5351_write(CLK0_CTRL, 0x4F | CLK_SRC_PLL_A);    // Strength 8mA
}

////////////////////////////////////////////////////////////////////////
// Set CLK1 output ON and to the specified frequency
// Frequency is in the range 1MHz to 150MHz
// Example: si5351aSetFrequency2(10000000);
// will set output CLK0 to 10MHz
//
// This example sets up PLL B
// and MultiSynth 1
// and produces the output on CLK1
////////////////////////////////////////////////////////////////////////

void si5351aSetFrequency2(uint32_t frequency){
  uint32_t pllFreq;
  //uint32_t xtalFreq = XTAL_FREQ;        // 2017/9/29
  uint32_t l;
  float f;
  uint8_t mult;
  uint32_t num;
  uint32_t denom;
  uint32_t divider;

  divider = 900000000 / frequency;        // Calculate the division ratio. 900,000,000 is the maximum internal
                                          // PLL frequency: 900MHz
  if (divider % 2) divider--;             // Ensure an even integer 
                                          //division ratio

  pllFreq = divider * frequency;          // Calculate the pllFrequency: 
                                          //the divider * desired output frequency

  mult = pllFreq / xtalFreq;              // Determine the multiplier to 
                                          //get to the required pllFrequency
  l = pllFreq % xtalFreq;                 // It has three parts:
  f = l;                                  // mult is an integer that must be in the range 15..90
  f *= 1048575;                           // num and denom are the fractional parts, the numerator and denominator
  f /= xtalFreq;                          // each is 20 bits (range 0..1048575)
  num = f;                                // the actual multiplier is mult + num / denom
  denom = 1048575;                        // For simplicity we set the denominator to the maximum 1048575

                                          // Set up PLL B with the calculated  multiplication ratio
  setupPLL(MSNB_ADDR, mult, num, denom);  
                                          // Set up MultiSynth divider 0, with the calculated divider.
                                          // The final R division stage can divide by a power of two, from 1..128.
                                          // reprented by constants SI_R_DIV1 to SI_R_DIV128 (see si5351a.h header file)
                                          // If you want to output frequencies below 1MHz, you have to use the
                                          // final R division stage
  setupMultisynth(MS1_ADDR, divider, R_DIV_1);
                                          // Reset the PLL. This causes a glitch in the output. For small changes to
                                          // the parameters, you don't need to reset the PLL, and there is no glitch
 // Si5351_write(PLL_RESET, 0xA0);
                                          // Finally switch on the CLK1 output
                                          // and set the MultiSynth0 input to be PLL B 
  Si5351_write(CLK1_CTRL, 0x4F | CLK_SRC_PLL_B);    // Strength 8mA
}
