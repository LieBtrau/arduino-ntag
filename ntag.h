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
    typedef enum{
        NC_REG,
        LAST_NDEF_BLOCK,
        SRAM_MIRROR_BLOCK,
        WDT_LS,
        WDT_MS,
        I2C_CLOCK_STR,
        REG_LOCK
    }REGISTER_NR;
    Ntag(DEVICE_TYPE dt);
    Ntag(DEVICE_TYPE dt, byte i2c_address);
    void detectI2cDevices();//Comes in handy when you accidentally changed the I²C address of the NTAG.
    bool begin();
    bool getSerialNumber(byte* sn);
    bool read(byte address, byte* pdata, byte length);
    bool write(byte address, byte* pdata, byte length);
    bool readUserMem(byte blockNr, byte *p_data, byte data_size);
    bool writeUserMem(byte blockNr, byte *p_data);
    bool read_register(REGISTER_NR regAddr, byte &value);
    bool write_register(REGISTER_NR regAddr, byte mask, byte regdat);
    void test();
    static const byte USERMEM_BLOCK1=0x01;
private:
    typedef enum{
        CONFIG=0x1,//BLOCK0 (putting this in a separate block type, because errors here can "brick" the device.)
        USERMEM=0x2,//EEPROM
        REGISTER=0x4,//Settings registers
        SRAM=0x8
    }BLOCK_TYPE;
    const byte DEFAULT_I2C_ADDRESS=0x55;
    const byte NTAG_BLOCK_SIZE=16;
    bool writeMemAddress(BLOCK_TYPE dt, byte addr);
    bool end_transmission(void);
    bool isAddressValid(BLOCK_TYPE dt, byte address);
    bool readBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data, byte data_size);
    bool writeBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data);
    byte _i2c_address;
    DEVICE_TYPE _dt;
};

#endif // NTAG_H
