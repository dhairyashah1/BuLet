## Firmware files for BLE baseband

## Beaglebone Black Implementation

For compiling the `beaglebone` source code on a Beaglebone black please follow the instructions [here](https://docs.google.com/document/d/1PNs6_6aTKkskBBkggTA4BKVbE6E85Iy5jcEpC5Fc3Zc/edit?usp=sharing)

## MSP430 Implementation

- `ble_baseband_5969` code works for MSP430FR5969 and can be ported to similar platforms easily. 
- The output waveform might generated on P1.2 might have minor bugs. 
- You need CCS Studio from TI to compile and execute this code.
- Thsi version is adapted from [kurisuuuu](https://github.com/kurisuuuu/BLE-Backscattering-Tag/tree/master) and yet to be improved to add support for additional modes like LE coded PHY.