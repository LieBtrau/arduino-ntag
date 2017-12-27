//[BSD License](https://github.com/don/Ndef/blob/master/LICENSE.txt) (c) 2013-2014, Don Coleman

#include "ntageepromadapter.h"

NtagEepromAdapter::NtagEepromAdapter(Ntag* ntag)
{
    _ntag=ntag;
}

bool NtagEepromAdapter::begin()
{
    NtagAdapter::begin();
    if((!_ntag->setSramMirrorRf(false, 0)) || (!_ntag->setFd_ReaderHandshake())){
        Serial.println("Can't initialize tag");
        return false;
    }
    return true;
}

bool NtagEepromAdapter::write(NdefMessage& m, unsigned int uiTimeout){
    if(!waitUntilRfDone(uiTimeout))
    {
        return false;
    }
    if (isUnformatted())
    {
        Serial.println(F("WARNING: Tag is not formatted."));
        return false;
    }
    if(!readCapabilityContainer())
    {
        return false;
    }

    messageLength  = m.getEncodedSize();
    ndefStartIndex = messageLength < 0xFF ? 2 : 4;
    calculateBufferSize();

    if(bufferSize>tagCapacity) {
#ifdef MIFARE_ULTRALIGHT_DEBUG
        Serial.print(F("Encoded Message length exceeded tag Capacity "));Serial.println(tagCapacity);
#endif
        return false;
    }

    uint8_t encoded[bufferSize];

    // Set message size.
    encoded[0] = 0x3;
    if (messageLength < 0xFF)
    {
        encoded[1] = messageLength;
    }
    else
    {
        encoded[1] = 0xFF;
        encoded[2] = ((messageLength >> 8) & 0xFF);
        encoded[3] = (messageLength & 0xFF);
    }
    m.encode(encoded+ndefStartIndex);
    // this is always at least 1 byte copy because of terminator.
    memset(encoded+ndefStartIndex+messageLength,0,bufferSize-ndefStartIndex-messageLength);
    encoded[ndefStartIndex+messageLength] = 0xFE; // terminator

#ifdef MIFARE_ULTRALIGHT_DEBUG
    Serial.print(F("messageLength "));Serial.println(messageLength);
    Serial.print(F("Tag Capacity "));Serial.println(tagCapacity);
    nfc->PrintHex(encoded,bufferSize);
#endif

    _ntag->writeEeprom(0,encoded,bufferSize);
    _ntag->setLastNdefBlock();
    _ntag->releaseI2c();
    //    for(int i=0;i<sizeof(buffer);i++){
    //        Serial.print(buffer[i], HEX);Serial.print(" ");
    //        if((i+1)%8==0)Serial.println();
    //    }
}

NfcTag NtagEepromAdapter::read(unsigned int uiTimeOut)
{
    if (isUnformatted())
    {
        Serial.println(F("WARNING: Tag is not formatted."));
        return NfcTag(uid, UID_LENGTH, NFC_FORUM_TAG_TYPE_2);
    }

    if(!readCapabilityContainer())
    {
        return NfcTag(uid, UID_LENGTH, NFC_FORUM_TAG_TYPE_2);;
    }
    findNdefMessage();
    calculateBufferSize();

    if (messageLength == 0) { // data is 0x44 0x03 0x00 0xFE
        NdefMessage message = NdefMessage();
        message.addEmptyRecord();
        return NfcTag(uid, UID_LENGTH, NFC_FORUM_TAG_TYPE_2, message);
    }

    bool success;
    uint8_t page;
    uint8_t index = 0;
    byte buffer[bufferSize];
    _ntag->readEeprom(0,buffer, bufferSize);
    NdefMessage ndefMessage = NdefMessage(&buffer[ndefStartIndex], messageLength);
    return NfcTag(uid, UID_LENGTH, NFC_FORUM_TAG_TYPE_2, ndefMessage);
}

// Mifare Ultralight can't be reset to factory state
// zero out tag data like the NXP Tag Write Android application
bool NtagEepromAdapter::clean()
{
    if(!readCapabilityContainer())
    {
        return false;
    }

    byte blocks = (tagCapacity / NTAG_BLOCK_SIZE);

    // factory tags have 0xFF, but OTP-CC blocks have already been set so we use 0x00
    byte data[16];
    memset(data,0x00,sizeof(data));

    for (int i = 0; i < blocks; i++) {
        #ifdef MIFARE_ULTRALIGHT_DEBUG
        Serial.print(F("Wrote page "));Serial.print(i);Serial.print(F(" - "));
        nfc->PrintHex(data, ULTRALIGHT_PAGE_SIZE);
        #endif
        if (!_ntag->writeEeprom(i,data,NTAG_BLOCK_SIZE)) {
            return false;
        }
    }
    return true;
}

bool NtagEepromAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}


bool NtagEepromAdapter::isUnformatted()
{
    const byte PAGE_4 = 0;//page 4 is base address of EEPROM from I²C perspective
    byte data[NTAG_PAGE_SIZE];
    bool success = _ntag->readEeprom(PAGE_4, data, NTAG_PAGE_SIZE);
    if (success)
    {
        return (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF);
    }
    else
    {
        Serial.print(F("Error. Failed read page 4"));
        return false;
    }
}

// page 3 has tag capabilities
bool NtagEepromAdapter::readCapabilityContainer()
{
    byte data[4];
    if (_ntag->getCapabilityContainer(data))
    {
        //http://apps4android.org/nfc-specifications/NFCForum-TS-Type-2-Tag_1.1.pdf
        if(data[0]!=0xE1)
        {
            return false;   //magic number
        }
        //NT3H1101 return 0x6D for data[2], which leads to 872 databytes, not 888.
        tagCapacity = data[2] * 8;
#ifdef MIFARE_ULTRALIGHT_DEBUG
        Serial.print(F("Tag capacity "));Serial.print(tagCapacity);Serial.println(F(" bytes"));
#endif

        // TODO future versions should get lock information
    }
    return true;
}

// buffer is larger than the message, need to handle some data before and after
// message and need to ensure we read full pages
void NtagEepromAdapter::calculateBufferSize()
{
    // TLV terminator 0xFE is 1 byte
    bufferSize = messageLength + ndefStartIndex + 1;

    if (bufferSize % NTAG_PAGE_SIZE != 0)
    {
        // buffer must be an increment of page size
        bufferSize = ((bufferSize / NTAG_PAGE_SIZE) + 1) * NTAG_PAGE_SIZE;
    }
}

// read enough of the message to find the ndef message length
void NtagEepromAdapter::findNdefMessage()
{
    byte data[16]; // 4 pages

    // the nxp read command reads 4 pages
    if (_ntag->readEeprom(0,data,16))
    {
        if (data[0] == 0x03)
        {
            messageLength = data[1];
            ndefStartIndex = 2;
        }
        else if (data[5] == 0x3) // page 5 byte 1
        {
            // TODO should really read the lock control TLV to ensure byte[5] is correct
            messageLength = data[6];
            ndefStartIndex = 7;
        }
    }

    #ifdef MIFARE_ULTRALIGHT_DEBUG
    Serial.print(F("messageLength "));Serial.println(messageLength);
    Serial.print(F("ndefStartIndex "));Serial.println(ndefStartIndex);
    #endif
}


