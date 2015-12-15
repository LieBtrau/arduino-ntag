#include "ntag.h"
#include "HardWire.h"
#include <stdio.h>

HardWire HWire(1, I2C_REMAP);// | I2C_BUS_RESET); // I2c1

Ntag::Ntag(DEVICE_TYPE dt): _dt(dt), _i2c_address(DEFAULT_I2C_ADDRESS)
{
}

Ntag::Ntag(DEVICE_TYPE dt, byte i2c_address): _dt(dt), _i2c_address(i2c_address)
{
}

bool Ntag::begin(){
    HWire.begin();
    HWire.beginTransmission(_i2c_address);
    return HWire.endTransmission()==0;
}

void Ntag::detectI2cDevices(){
    for(byte i=0;i<0x80;i++){
        HWire.beginTransmission(i);
        if(HWire.endTransmission()==0)
        {
            Serial.print("Found I²C device on : 0x");
            Serial.println(i,HEX);
        }
    }
}

bool Ntag::getSerialNumber(byte* sn){
    byte data[7];
    if(!readBlock(CONFIG, 0,data,7)){
        return false;
    }
    if(data[0]!=4){
        return false;
    }
    memcpy(sn, data,7);
    return true;
}

bool Ntag::setSramMemMap(bool bEnable){
    //Mirror SRAM to bottom of USERMEM (avoid firmware change in NFC-reader)
    if(!writeRegister(SRAM_MIRROR_BLOCK,0xFF,0x01)){
        return false;
    }
    //disable pass-through mode
    //enable/disable SRAM memory mirror
    if(!writeRegister(NC_REG, 0x42, bEnable ? 0x02 : 0x00)){
        return false;
    }
    return true;
}

bool Ntag::write(byte address, byte* pdata, byte length)
{
    byte readbuffer[NTAG_BLOCK_SIZE];
    byte writeLength;
    byte* wptr=pdata;

    writeLength=min(NTAG_BLOCK_SIZE, (address % NTAG_BLOCK_SIZE) + length);
    if(address % NTAG_BLOCK_SIZE !=0)
    {
        //start address doesn't point to start of block, so we must read the bytes that precede the address range to
        //be written.
        if(!readUserMem(USERMEM_BLOCK1 + (address/NTAG_BLOCK_SIZE),readbuffer, writeLength))
        {
            return false;
        }
        writeLength-=address % NTAG_BLOCK_SIZE;
        memcpy(readbuffer + (address % NTAG_BLOCK_SIZE), pdata, writeLength);
        if(!writeUserMem(USERMEM_BLOCK1 + (address/NTAG_BLOCK_SIZE),readbuffer))
        {
            return false;
        }
        wptr+=writeLength;
    }
    else
    {
        if(!writeUserMem(USERMEM_BLOCK1 + (address/NTAG_BLOCK_SIZE),wptr))
        {
            return false;
        }
        wptr+=NTAG_BLOCK_SIZE;
    }
    for(byte i=(address/NTAG_BLOCK_SIZE)+1;wptr<pdata+length;i++)
    {
        writeLength=(pdata+length-wptr > NTAG_BLOCK_SIZE ? NTAG_BLOCK_SIZE : pdata+length-wptr);
        if(writeLength!=NTAG_BLOCK_SIZE){
            if(!readUserMem(USERMEM_BLOCK1 + i,readbuffer, NTAG_BLOCK_SIZE))
            {
                return false;
            }
            memcpy(readbuffer, wptr, writeLength);
        }
        if(!writeUserMem(USERMEM_BLOCK1 + i, writeLength==NTAG_BLOCK_SIZE ? wptr : readbuffer))
        {
            return false;
        }
        wptr+=writeLength;
    }
    return true;
}

bool Ntag::read(byte address, byte* pdata,  byte length)
{
    byte readbuffer[NTAG_BLOCK_SIZE];
    byte readLength;
    byte* wptr=pdata;

    readLength=min(NTAG_BLOCK_SIZE, (address % NTAG_BLOCK_SIZE) + length);
    if(!readUserMem(USERMEM_BLOCK1 + (address/NTAG_BLOCK_SIZE),readbuffer, readLength))
    {
        return false;
    }
    readLength-=address % NTAG_BLOCK_SIZE;
    memcpy(wptr,readbuffer + (address % NTAG_BLOCK_SIZE), readLength);
    wptr+=readLength;
    for(byte i=(address/NTAG_BLOCK_SIZE)+1;wptr<pdata+length;i++)
    {
        readLength=(pdata+length-wptr > NTAG_BLOCK_SIZE ? NTAG_BLOCK_SIZE : pdata+length-wptr);
        if(!readUserMem(USERMEM_BLOCK1 + i, wptr, readLength))
        {
            return false;
        }
        wptr+=readLength;
    }
    return true;
}

bool Ntag::readUserMem(byte blockNr, byte *p_data, byte data_size)
{
    return readBlock(USERMEM, blockNr, p_data, data_size);
}

bool Ntag::writeUserMem(byte blockNr, byte *p_data)
{
    return writeBlock(USERMEM, blockNr, p_data);
}

bool Ntag::readBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data, byte data_size)
{
    if(data_size>NTAG_BLOCK_SIZE || !writeMemAddress(bt, memBlockAddress)){
        return false;
    }
    if(!end_transmission()){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,data_size)!=data_size){
        return false;
    }
    byte i=0;
    while(HWire.available())
    {
        p_data[i++] = HWire.read();
    }
    return i==data_size;
}

bool Ntag::writeBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data)
{
    if(!writeMemAddress(bt, memBlockAddress)){
        return false;
    }
    for (int i=0; i<NTAG_BLOCK_SIZE; i++)
    {
        if(HWire.write(p_data[i])!=1){
            break;
        }
    }
    if(!end_transmission()){
        return false;
    }
    switch(bt){
    case CONFIG:
    case USERMEM:
        delay(5);//16 bytes (one block) written in 4.5 ms (EEPROM)
        break;
    case REGISTER:
    case SRAM:
        delay_us(500);//0.4 ms (SRAM - Pass-through mode) including all overhead
        break;
    }
    return true;
}

bool Ntag::readRegister(REGISTER_NR regAddr, byte& value)
{
    value=0;
    bool bRetVal=true;
    if(regAddr>6 || !writeMemAddress(REGISTER, 0xFE)){
        return false;
    }
    if(HWire.write(regAddr)!=1){
        bRetVal=false;
    }
    if(!end_transmission()){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,(byte)1)!=1){
        return false;
    }
    value=HWire.read();
    return bRetVal;
}

bool Ntag::writeRegister(REGISTER_NR regAddr, byte mask, byte regdat)
{
    bool bRetVal=false;
    if(regAddr>7 || !writeMemAddress(REGISTER, 0xFE)){
        return false;
    }
    if (HWire.write(regAddr)==1 &&
            HWire.write(mask)==1 &&
            HWire.write(regdat)==1){
        bRetVal=true;
    }
    return end_transmission() && bRetVal;
}

bool Ntag::writeMemAddress(BLOCK_TYPE dt, byte addr)
{
    if(!isAddressValid(dt, addr)){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    return HWire.write(addr)==1;
}

bool Ntag::end_transmission(void)
{
    return HWire.endTransmission()==0;
    //I2C_LOCKED must be either reset to 0b at the end of the I2C sequence or wait until the end of the watch dog timer.
}

bool Ntag::isAddressValid(BLOCK_TYPE type, byte address){
    switch(type){
    case CONFIG:
        if(address!=0){
            return false;
        }
        break;
    case USERMEM:
        switch (_dt) {
        case NTAG_I2C_1K:
            if(address < 1 || address > 0x38){
                return false;
            }
            break;
        case NTAG_I2C_2K:
            if(address < 1 || address > 0x78){
                return false;
            }
            break;
        default:
            return false;
        }
        break;
    case SRAM:
        if(address < 0xF8 || address > 0xFB){
            return false;
        }
        break;
    case REGISTER:
        switch (_dt) {
        //todo: check if writing 0x3A also requires write_register instead of write function.
        case NTAG_I2C_1K:
            if(address != 0x3A && address != 0xFE){
                return false;
            }
            break;
        case NTAG_I2C_2K:
            if(address != 0x7A && address != 0xFE){
                return false;
            }
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}
