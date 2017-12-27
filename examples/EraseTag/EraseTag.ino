// Erases a NFC tag by writing an empty NDEF message
// The NDEF-messages is wrapped in a TLV-block.
//  T: 03       : TLV Block Type = NDEF-message
//  L: 03       : Length of value field = 3 bytes
//  V: D0 00 00 : Value field
//      NDEF-message
//          Record header : D0 = 1101000b
//              bits 2-0    : TNF = type name field = 0 : empty record
//          Type length     : 00
//          Payload length  : 00
//  TLV2: 0xFE = terminator TLV

#include "ntageepromadapter.h"
#define HARDI2C

Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagEepromAdapter ntagAdapter(&ntag);

void setup(void)
{
    Serial.begin(115200);
    Serial.println("NFC Tag Eraser");
    ntagAdapter.begin();
    if (ntagAdapter.erase())
    {
        Serial.println("\nSuccess, tag contains an empty record.");
    } else
    {
        Serial.println("\nUnable to erase tag.");
    }
}

void loop(){}
