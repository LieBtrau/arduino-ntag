#include "ntageepromadapter.h"
#define HARDI2C

Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagEepromAdapter ntagAdapter(&ntag);

void setup()
{
      Serial.begin(115200);
      Serial.println("NDEF Writer");
      ntagAdapter.begin();
      NdefMessage message = NdefMessage();
      message.addUriRecord("http://arduino.cc");

      if (ntagAdapter.write(message)) {
        Serial.println("Success. Try reading this tag with your phone.");
      } else {
        Serial.println("Write failed.");
      }
}

void loop() {}
