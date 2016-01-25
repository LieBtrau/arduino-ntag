#ifndef NTAGSRAMADAPTER_H
#define NTAGSRAMADAPTER_H

#include "ntag.h"
#include "NfcTag.h"

class NtagSramAdapter
{
public:
    NtagSramAdapter(Ntag* ntag);
    void begin(boolean verbose=true);
    boolean readerPresent(unsigned long timeout=0);
    NfcTag read(uint uiTimeOut=0);
    bool rfBusy();
    bool write(NdefMessage& message, uint uiTimeout=0);
    // erase tag by writing an empty NDEF record
    boolean erase();
    // format a tag as NDEF
    boolean format();
    // reset tag back to factory state
    boolean clean();
    bool getUid(byte *uidin, unsigned int uidLength);
    byte getUidLength();
private:
    static const byte SRAM_SIZE=64;
    static const byte UID_LENGTH=7;
    bool decodeTlv(byte *data, int &messageLength, int &messageStartIndex);
    bool waitUntilRfDone(uint uiTimeOut);
    int getNdefStartIndex(byte *data);
    Ntag* _ntag;
    byte uid[UID_LENGTH];  // Buffer to store the returned UID
};

#endif // NTAGSRAMADAPTER_H
