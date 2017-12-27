#include "ntageepromadapter.h"
#define HARDI2C

Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagEepromAdapter ntagAdapter(&ntag);

void setup(void) {
    Serial.begin(115200);
    Serial.println("NDEF Reader");
    ntagAdapter.begin();
    NfcTag tag = ntagAdapter.read();
    tag.print();
}

void loop(void) {}
