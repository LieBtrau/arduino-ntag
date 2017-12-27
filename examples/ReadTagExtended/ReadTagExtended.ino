#include "ntageepromadapter.h"
#define HARDI2C

Ntag ntag(Ntag::NTAG_I2C_1K,2,5);
NtagEepromAdapter ntagAdapter(&ntag);

void setup(void) {
    Serial.begin(115200);
    Serial.println("NDEF Reader");
    ntagAdapter.begin();
    NfcTag tag = ntagAdapter.read();
    Serial.println(tag.getTagType());
    Serial.print("UID: ");Serial.println(tag.getUidString());

    if (tag.hasNdefMessage()) // every tag won't have a message
    {

        NdefMessage message = tag.getNdefMessage();
        Serial.print("\nThis NFC Tag contains an NDEF Message with ");
        Serial.print(message.getRecordCount());
        Serial.print(" NDEF Record");
        if (message.getRecordCount() != 1)
        {
            Serial.print("s");
        }
        Serial.println(".");

        // cycle through the records, printing some info from each
        int recordCount = message.getRecordCount();
        for (int i = 0; i < recordCount; i++)
        {
            Serial.print("\nNDEF Record ");Serial.println(i+1);
            NdefRecord record = message.getRecord(i);
            // NdefRecord record = message[i]; // alternate syntax

            Serial.print("  TNF: ");Serial.println(record.getTnf());
            Serial.print("  Type: ");Serial.println(record.getType()); // will be "" for TNF_EMPTY

            // The TNF and Type should be used to determine how your application processes the payload
            // There's no generic processing for the payload, it's returned as a byte[]
            int payloadLength = record.getPayloadLength();
            byte payload[payloadLength];
            record.getPayload(payload);

            // Print the Hex and Printable Characters
            Serial.print("  Payload (HEX): ");
            PrintHexChar(payload, payloadLength);

            // Force the data into a String (might work depending on the content)
            // Real code should use smarter processing
            String payloadAsString = "";
            for (int c = 0; c < payloadLength; c++)
            {
                payloadAsString += (char)payload[c];
            }
            Serial.print("  Payload (as String): ");
            Serial.println(payloadAsString);

            // id is probably blank and will return ""
            String uid = record.getId();
            if (uid != "")
            {
                Serial.print("  ID: ");Serial.println(uid);
            }
        }
    }
}

void loop(void) {}
