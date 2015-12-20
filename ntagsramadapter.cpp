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
    //    NdefMessage message = NdefMessage();
    //    message.addUriRecord("http://arduino.cc");
    uint8_t encoded[message.getEncodedSize()];
    message.encode(encoded);
    uint8_t buffer[3 + sizeof(encoded)];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0x3;
    buffer[1] = sizeof(encoded);
    memcpy(&buffer[2], encoded, sizeof(encoded));
    buffer[2+sizeof(encoded)] = 0xFE; // terminator
    _ntag->writeSram(0,buffer,3 + sizeof(encoded));
    _ntag->setLastNdefBlock();
}
