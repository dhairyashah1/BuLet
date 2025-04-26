
/**********************************************
* BeagleBone PRU Backscatter
* BLE backscatter
* Adaptation of the code from below:
* Authors : Ambuj Varshney < ambuj_varshney@it.uu.se >
* (C) 2016 Uppsala Networked Objects (UNO)
************************************************/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

// The structure keeps track of the payload to be backscattered
// also holds the control variable useful for the backscatter.

struct blePacket{
      // 1 byte: preamble for the BLE packet
      uint8_t u8preamble;

      // 4 bytes: MAC Accesss address
      uint8_t access_address[4];

      // 42 bytes: (2 - 39 bytes) PDU + (3 bytes) CRC
            // 2 bytes: header
                  // 4 bites: PDU type
                  // 1 bit: RFU
                  // 1 bit: ChSel =>  Set to 1 if LE Channel Selection Algorithm #2 is supported.
                  // 1 bit: TxAdd => (Tx Address): 0 or 1, depending on the transmitter address is public or random.
                  // 1 bit: RxAdd =>  (Rx Address): 0 or 1, depending on the receiver address is public or random.
                  // 1 byte: Length of the payload
            // 37 bytes: payload
                  // 6 bytes: AdvA: Bluetooth address of the advertising device
                  // 31 bytes: AdvData: Advertisement data packet
      uint8_t blePDU[42];

      // Control variable: Payload length
      unsigned char u8PayloadLen;
};

// Delays measured in PRU toggle for the TWO BLE freuqnecies

// BLE Frequencies with Carrier shift of 5 MHz, Freq 1 : 5.5 MHz, Freq 2 : 4.5 MHz
//#define BLE_FREQ1 110
//#define BLE_FREQ2 90

// 4 MHz, config 1
//#define BLE_FREQ1 130
//#define BLE_FREQ2 110

// 2 MHz shift, Dev - 275 KHz
#define BLE_FREQ1 290 //
#define BLE_FREQ2 220 //

// Predefined bytes reserved in BLE packet for header and other PDU realated data
#define BLE_PAYLOAD_PREDEF 11
// This keeps track of the maximum size in bytes of the BLE mac address
#define BLE_MAC_ADDRESS 6
// use PRU 0 for the Backscatter
#define PRU_NUM 0 /* which of the two PRUs are we using? */

#define MAX_PAYLOAD 30
// BLE payload generation related function
void btLeCrc(const uint8_t* , uint8_t , uint8_t* );
uint8_t  swapbits(uint8_t );
void btLeWhiten(uint8_t* , uint8_t , uint8_t );
void generate_ble_adv_payload(struct blePacket *, char [], char [], uint8_t );



