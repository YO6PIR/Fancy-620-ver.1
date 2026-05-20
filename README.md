<img width="2624" height="1632" alt="image" src="https://github.com/user-attachments/assets/bd2f75dd-7304-4996-a4cf-e9f8724bd2cf" />

The ingenuity of this project lies in the fact that it displays a "Vintage" type analog scale with an indicator needle for frequency variation.
Features of this controller:
Processor: STM32F103C8T6, 64KB Flash, 20KB RAM
VFO-DDS: Si5351, VFO, BFO, IF-SHIFT, RIT, scala gradata mare
Display: ILI9341 SPI
LCD Library: Ucglib.h -HWSPI
MEMORY: EEProm I2C 24C02 - 2kB RAM, sau mai mare
Based on 'JAN2KD 2016.10.19 Multi Band DDS VFO Ver3.1 and JA2GQP-2020

Features of this controller:
6 frequency bands 80,40,20,17,15,10m........[Key1] short
VFO A<->B switch........................................[Key1] long
BFO adjustment on each mode at startup.............[Key2] held down
LSB, USB and CW working modes .......................[Key2] short
AGC ON/OFF(SLOW/FAST)................................[Key2] lung
AMPLI +20dB ................................................ [Key3] short
ATT ON/OFF................................................. [Key3] lung
RIT with graduated scale ON/OFF..........................[Key4]short
Linear scale on the center of the screen ON/OFF.........[Key4]long
LOCK KNOB ....................................................[ENC] long
STEP x10, x100, x1K......................................[ENC] short
XTALL oscillator adjustment at startup with ............[ENC] held down
BCD code band switching output with CD4028 decoder
When changing step, rounds the VFO by the set STEP value.
Automatically saves current data in EEprom, 2s after frequency or band change
Analog bargraph on S-meter and POWER-meter
Communication UI messages at the bottom of the screen
Supply voltage monitored on screen Max=19Vcc
Final radiator temperature measurement with external 10K Thermistor
On CW transmission, a 700Hz tone is generated + FREQ_VFO
