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
    bool write(NdefMessage& message);
    // erase tag by writing an empty NDEF record
    boolean erase();
    // format a tag as NDEF
    boolean format();
    // reset tag back to factory state
    boolean clean();
private:
    Ntag* _ntag;
    byte uid[7];  // Buffer to store the returned UID
};

#endif // NTAGSRAMADAPTER_H
