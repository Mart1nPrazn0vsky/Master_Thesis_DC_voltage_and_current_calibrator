Gateware for Artix-7 FPGA (Cmod A7-35T) to control Low-voltage module of "voltage and current calibrator"
by Martin Praznovsky, 2025

FPGA controls digital-to-analog converter DAC11001B, 3 relays frow selecting an output range and disconnecting output, LED diodes on the front panel of the device. Module works in 3 different modes:
DC mode with standard resolution - 20 bits
DC mode with increased resolution (dithering) - 24 bits
AC mode - 20 bits, max. amplitude is x80000
FPGA is controled via UART line, which writes data into 4 control registers:

-- SYSTEM REGISTERS
    
--  register G
--  -----------------------------------------------------------------
--  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
--  -----------------------------------------------------------------
--  | - | - | - | - | - | - | - | - |             byte              |
--  -----------------------------------------------------------------
--  byte       - if byte = '?', send name + content of all registers

--  register H
--  -----------------------------------------------------------------
--  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
--  -----------------------------------------------------------------
--  | - |L4R|L4G|L3R|L3G|L2R|L2G|L1R|L1G|OL2|OL1|AC |DIT| R3| R2| R1|
--  -----------------------------------------------------------------
--  L4R        - LED 4 red (1 = LED ON, 0 = LED OFF)
--  L4G        - LED 4 green (1 = LED ON, 0 = LED OFF)
--  L3R        - LED 3 red (1 = LED ON, 0 = LED OFF)  
--  L3G        - LED 3 green (1 = LED ON, 0 = LED OFF)
--  L2R        - LED 2 red (1 = LED ON, 0 = LED OFF)  
--  L2G        - LED 2 green (1 = LED ON, 0 = LED OFF)
--  L1R        - LED 4 red (1 = LED ON, 0 = LED OFF)  
--  L1G        - LED 4 green (1 = LED ON, 0 = LED OFF)
--  OL2        - backlight for output binding posts 1 (1 = LED ON, 0 = LED OFF)
--  OL1        - backlight for output binding posts 2 (1 = LED ON, 0 = LED OFF)
--  AC         - 0 = AC mode OFF, 1 = AC mode ON
--  DIT        - 0 = dithering OFF, 1 = dithering ON
--  Rx         - 0 = relay in default state, 1 = relay switched
	
--  register I
--  ---------------------------------------------------------------------------------------------------------------------------------
--  |31 |30 |29 |28 |27 |26 |25 |24 |23 |22 |21 |20 |19 |18 |17 |16 |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
--  ---------------------------------------------------------------------------------------------------------------------------------
--  | - | - | - | - | - | - | - | - |                                 voltage                                       |   dithering   |
--  ---------------------------------------------------------------------------------------------------------------------------------
--  voltage    - 20-bit code for DAC in DC mode, in DC mode with dithering top 20 bits of code, in AC mode amplitude (max. x80000)
--  dithering  - 4 lowest bits of 24-bit code for DC generation when dithering is ON

--  register J
--  ---------------------------------------------------------------------------------------------------------------------------------
--  |31 |30 |29 |28 |27 |26 |25 |24 |23 |22 |21 |20 |19 |18 |17 |16 |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
--  ---------------------------------------------------------------------------------------------------------------------------------
--  |                                                  frequency tuning word                                                        |
--  ---------------------------------------------------------------------------------------------------------------------------------
--  frequency tuning word - FTW, DDS adds FTW to phase accumulator after every sample (X"FFFFFFFF" = 360°)
