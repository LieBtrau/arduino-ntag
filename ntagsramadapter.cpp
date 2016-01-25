#include "ntagsramadapter.h"
#include "Ndef.h"

NtagSramAdapter::NtagSramAdapter(Ntag *ntag)
{
    _ntag=ntag;
}

void NtagSramAdapter::begin(bool verbose){
    _ntag->begin();
    _ntag->getUid(uid, sizeof(uid));
    //Mirror SRAM to bottom of USERMEM
    //  this avoids firmware change in NFC-reader library
    //  the disadvantage is that the tag has to poll to over I²C to check if the memory is still locked to the RF-side.
    //Set FD_pin to function as handshake signal
    if((!_ntag->setSramMirrorRf(true, 0x01)) || (!_ntag->setFd_ReaderHandshake())){
        Serial.println("Can't initialize tag");
    }
}

bool NtagSramAdapter::write(NdefMessage& message, uint uiTimeout){
    if(!waitUntilRfDone(uiTimeout))
    {
        return false;
    }
    byte encoded[message.getEncodedSize()];
    message.encode(encoded);
    if(3 + sizeof(encoded) > SRAM_SIZE){
        return false;
    }
    byte buffer[3 + sizeof(encoded)];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0x3;
    buffer[1] = sizeof(encoded);
    memcpy(&buffer[2], encoded, sizeof(encoded));
    buffer[2+sizeof(encoded)] = 0xFE; // terminator
    _ntag->writeSram(0,buffer,3 + sizeof(encoded));
    _ntag->setLastNdefBlock();
    _ntag->releaseI2c();
//    for(int i=0;i<sizeof(buffer);i++){
//        Serial.print(buffer[i], HEX);Serial.print(" ");
//        if((i+1)%8==0)Serial.println();
//    }
}

bool NtagSramAdapter::rfBusy(){
    //wait until FD is high, indicating that RF-reading is done
    return _ntag->rfBusy();
}

bool NtagSramAdapter::readerPresent(unsigned long timeout)
{
    unsigned long startTime=millis();
    do
    {
        if(_ntag->readerPresent())
        {
            return true;
        }
    }while(millis()<startTime+timeout);

    return false;
}


NfcTag NtagSramAdapter::read(uint uiTimeOut){
    int messageStartIndex = 0;
    int messageLength = 0;
    byte buffer[SRAM_SIZE];

    if(!waitUntilRfDone(uiTimeOut)){
        return NfcTag(uid,UID_LENGTH,"NOT READY2");
    }
    if(!_ntag->readSram(0,buffer,SRAM_SIZE)){
        return NfcTag(uid,UID_LENGTH,"ERROR");
    }
//    for(int i=0;i<SRAM_SIZE;i++){
//        Serial.print(buffer[i], HEX);Serial.print(" ");
//        if((i+1)%8==0)Serial.println();
//    }
//    Serial.println();
    if (!decodeTlv(buffer, messageLength, messageStartIndex)) {
        return NfcTag(uid, UID_LENGTH, "ERROR");
    }
    return NfcTag(uid, UID_LENGTH, "NTAG", &buffer[messageStartIndex], messageLength);
}

bool NtagSramAdapter::waitUntilRfDone(uint uiTimeOut)
{
    if(uiTimeOut>0)
    {
        unsigned long ulStartTime=millis();
        while(millis() < ulStartTime+uiTimeOut)
        {
            if(!_ntag->rfBusy())
            {
                return true;
            }
        }
    }
    return !_ntag->rfBusy();
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

byte NtagSramAdapter::getUidLength()
{
    return UID_LENGTH;
}

bool NtagSramAdapter::getUid(byte *uidin, unsigned int uidLength)
{
    memcpy(uidin, uid, UID_LENGTH < uidLength ? UID_LENGTH : uidLength);
    return true;
}

