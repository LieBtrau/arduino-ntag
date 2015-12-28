#include "ntagsramadapter.h"
#include "Ndef.h"

NtagSramAdapter::NtagSramAdapter(Ntag *ntag)
{
    _ntag=ntag;
}

void NtagSramAdapter::begin(bool verbose){
    _ntag->begin();
    _ntag->getSerialNumber(uid);
    //Mirror SRAM to bottom of USERMEM (avoids firmware change in NFC-reader)
    _ntag->setSramMirrorRf(true, 0x01);
}

bool NtagSramAdapter::write(NdefMessage& message){
    byte encoded[message.getEncodedSize()];
    message.encode(encoded);
    byte buffer[3 + sizeof(encoded)];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0x3;
    buffer[1] = sizeof(encoded);
    memcpy(&buffer[2], encoded, sizeof(encoded));
    buffer[2+sizeof(encoded)] = 0xFE; // terminator
    _ntag->writeSram(0,buffer,3 + sizeof(encoded));
    _ntag->setLastNdefBlock();
}

NfcTag NtagSramAdapter::read(){
    int messageStartIndex = 0;
    int messageLength = 0;
    byte buffer[64];

    if(!_ntag->readSram(0,buffer,64)){
        return NfcTag(uid,UID_LENGTH,"ERROR");
    }
    if (!decodeTlv(buffer, messageLength, messageStartIndex)) {
        return NfcTag(uid, UID_LENGTH, "ERROR");
    }
    return NfcTag(uid, UID_LENGTH, "NTAG", &buffer[messageStartIndex], messageLength);
}

// Decode the NDEF data length from the Mifare TLV
// Leading null TLVs (0x0) are skipped
// Assuming T & L of TLV will be in the first block
// messageLength and messageStartIndex written to the parameters
// success or failure status is returned
//
// { 0x3, LENGTH }
bool NtagSramAdapter::decodeTlv(byte *data, int &messageLength, int &messageStartIndex)
{
    int i = getNdefStartIndex(data);

    if (i < 0 || data[i] != 0x3)
    {
        Serial.println(F("Error. Can't decode message length."));
        return false;
    }
    else
    {
        messageLength = data[i+1];
        messageStartIndex = i + 2;
    }

    return true;
}

// skip null tlvs (0x0) before the real message
// technically unlimited null tlvs, but we assume
// T & L of TLV in the first block we read
int NtagSramAdapter::getNdefStartIndex(byte *data)
{

    for (int i = 0; i < 16; i++)
    {
        if (data[i] == 0x0)
        {
            // do nothing, skip
        }
        else if (data[i] == 0x3)
        {
            return i;
        }
        else
        {
            Serial.print("Unknown TLV ");Serial.println(data[i], HEX);
            return -2;
        }
    }

    return -1;
}
