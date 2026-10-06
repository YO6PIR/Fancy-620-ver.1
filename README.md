<img width="800" alt="Fancy-620-Ver.1 transceiver controller" src="https://github.com/user-attachments/assets/bd2f75dd-7304-4996-a4cf-e9f8724bd2cf" />

# Fancy-620 Ver.1

A compact HF transceiver controller with vintage-style analog display.

Fancy-620 Ver.1 is an STM32-based radio controller featuring a classic analog frequency scale with needle indicator, modern VFO/BFO control, and multi-band support. The design combines retro aesthetics with digital precision, making it suitable for amateur radio enthusiasts building or modifying HF transceivers.

---

## Overview

This controller replaces traditional analog tuning mechanisms with a digital VFO system while preserving the familiar "vintage" look of an analog frequency dial.

It provides:

- multi-band HF operation (6 bands: 80m, 40m, 20m, 17m, 15m, 10m)
- VFO A/B switching for frequency memory
- independent BFO for SSB and CW modes
- step tuning (x10, x100, x1k)
- RIT (receiver incremental tuning)
- automatic EEPROM storage
- S-meter and power meter display
- AGC control (ON/OFF, SLOW/FAST)
- transmit amplifier (+20 dB) and attenuator control
- temperature monitoring
- CW tone generation at 700 Hz

---

## Hardware

| Component | Specification |
|---|---|
| Microcontroller | STM32F103C8T6 (64 KB Flash, 20 KB RAM) |
| Frequency Synthesizer | Si5351 (VFO, BFO, IF-SHIFT, RIT) |
| Display | ILI9341 color TFT, SPI interface |
| Display Library | Ucglib.h with hardware SPI |
| Memory | I2C EEPROM 24C02 (2 kB or larger) |
| Band Switching | CD4028 decoder for BCD code output |
| Monitoring | Supply voltage (up to 19V) and final radiator temperature (10K thermistor) |

---

## Operating Modes

The controller supports three working modes:

- **LSB** — lower sideband for HF reception
- **USB** — upper sideband for HF reception
- **CW** — continuous wave with 700 Hz tone generation

Mode selection is via a dedicated button.

---

## Key Functions

### Frequency Control

- **Band Selection** — 6 HF bands accessible via [Key1] short press
- **VFO A / VFO B** — dual VFO memory with [Key1] long press
- **Step Size** — selectable x10, x100, x1k via encoder short press
- **Frequency Rounding** — VFO automatically rounds to the selected step value
- **LOCK Mode** — encoder lock via encoder long press

### Signal Processing

- **BFO Adjustment** — independent BFO tuning on each mode at startup via [Key2] held down
- **IF-SHIFT** — frequency offset control for noise reduction
- **RIT Control** — receiver incremental tuning with graduated scale via [Key4] short press
- **AGC Control** — AGC on/off with SLOW/FAST selection via [Key2] long press
- **Amplifier** — +20 dB amplifier option via [Key3] short press
- **Attenuator** — RF attenuator on/off via [Key3] long press

### Calibration

- **Crystal Oscillator Adjustment** — XTALL calibration at startup via encoder held down
- **BFO Calibration** — independent per mode adjustment
- **Band Switching** — BCD code output for external band selection decoder

---

## Display Features

- **Vintage Analog Scale** — large graduated frequency dial with smooth needle indicator
- **Linear Scale** — optional center-screen linear scale for fine tuning via [Key4] long press
- **S-Meter** — analog bargraph display of received signal strength
- **Power Meter** — analog bargraph of transmitted power
- **Status Messages** — communication feedback at screen bottom
- **Voltage Monitor** — supply voltage display (0–19V)
- **Temperature Display** — final stage radiator temperature

---

## Memory Management

- **Automatic Save** — frequency, band, and mode settings saved to EEPROM 2 seconds after any change
- **Persistent Settings** — all user settings recalled on power-up
- **VFO Memory** — two independent VFO frequencies stored per band

---

## Project Structure

```text
Fancy-620-ver.1/
├── Fancy620_STM32_Ver5.ino           # Main firmware
├── Fancy620_STM32F103C8T6_Ver5.bin   # Compiled binary
├── si5351.ino                         # Si5351 control
├── scala.ino                          # Frequency scale rendering
├── eprom.ino                          # EEPROM management
├── Version 5 Readme.txt               # Additional documentation
└── README.md
```

---

## Use Cases

- HF transceiver controller for homebrew radio projects
- replacement controller for vintage radio modifications
- multi-band VFO system for SSB and CW operation
- educational amateur radio platform
- compact radio station with frequency memory and band switching

---

## Building and Uploading

### Prerequisites

- Arduino IDE with STM32 board support
- USB-to-serial or ST-Link programmer
- Si5351 library
- Ucglib graphics library

### Compilation

1. Install STM32 board package in Arduino IDE
2. Select board: **STM32F1xx Series** → **Generic STM32F103C8**
3. Configure upload method
4. Compile and upload the sketch

---

## Status

Fancy-620 Ver.1 is a functional HF transceiver controller suitable for amateur radio use.

The firmware is stable for:

- frequency synthesis and tuning
- multi-mode SSB and CW operation
- S-meter and power monitoring
- EEPROM storage and recall

---

## Credits

Based on:
- JAN2KD 2016.10.19 Multi Band DDS VFO Ver3.1
- JA2GQP-2020

Developed and enhanced by Ovidiu — YO6PIR

---

## License

This project is provided for educational and personal radio experimentation use.

See LICENSE for full licensing terms.

---

**Firmware Version:** 5  
**Last Updated:** October 2026  
**Status:** Active Development
