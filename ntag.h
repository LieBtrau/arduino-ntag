#ifndef NTAG_H
#define NTAG_H

#include "Arduino.h"

class Ntag
{
public:
    typedef enum{
        NTAG_I2C_1K,
        NTAG_I2C_2K
    }DEVICE_TYPE;
    Ntag(DEVICE_TYPE dt);
    Ntag(DEVICE_TYPE dt, byte i2c_address);
    bool begin();
    bool getSerialNumber(byte* sn);
    bool read(byte mema, byte *p_data, byte data_size);
    byte write(byte mema, byte *p_data, byte data_size);
    bool read_register(byte regAddr, byte &value);
    bool write_register(byte regAddr, byte mask, byte regdat);
private:
    typedef enum{
        USERDATA,
        REGISTER
    }BLOCK_TYPE;
    const byte DEFAULT_I2C_ADDRESS=0x55;
    const byte NTAG_BLOCK_SIZE=16;
    bool writeMemAddress(BLOCK_TYPE dt, byte addr);
    bool end_transmission(void);
    bool isAddressValid(BLOCK_TYPE dt, byte address);
    byte _i2c_address;
    DEVICE_TYPE _dt;
};

#endif // NTAG_H
