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
    NfcTag read();
    bool rfReadingDone();
    bool write(NdefMessage& message);
    // erase tag by writing an empty NDEF record
    boolean erase();
    // format a tag as NDEF
    boolean format();
    // reset tag back to factory state
    boolean clean();
    bool getUid(byte *uidin, unsigned int uidLength);
    byte getUidLength();
private:
    static const byte UID_LENGTH=7;
    bool decodeTlv(byte *data, int &messageLength, int &messageStartIndex);
    int getNdefStartIndex(byte *data);
    Ntag* _ntag;
    byte uid[UID_LENGTH];  // Buffer to store the returned UID
};

#endif // NTAGSRAMADAPTER_H
