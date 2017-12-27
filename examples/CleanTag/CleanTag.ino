// Clean resets a tag back to factory-like state
// For Mifare Classic, tag is zero'd and reformatted as Mifare Classic
// For Mifare Ultralight, tags is zero'd and left empty

#include "ntageepromadapter.h"
#define HARDI2C

Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagEepromAdapter ntagAdapter(&ntag);


void setup(void)
{
    Serial.begin(115200);
    Serial.println("NFC Tag Cleaner");
    ntagAdapter.begin();
    bool success = ntagAdapter.clean();
    if (success)
    {
        Serial.println("\nSuccess, tag restored to factory state.");
    } else
    {
        Serial.println("\nError, unable to clean tag.");
    }
}

void loop(){}
