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
    if(!readchip(CONFIG, 0,data,7)){
        return false;
    }
    if(data[0]!=4){
        return false;
    }
    memcpy(sn, data,7);
    return true;
}

bool Ntag::readUserMem(byte memBlockAddress, byte *p_data, byte data_size)
{
    return readchip(USERMEM, memBlockAddress, p_data, data_size);
}

bool Ntag::writeUserMem(byte memBlockAddress, byte *p_data, byte data_size)
{
    return writechip(USERMEM, memBlockAddress, p_data, data_size);
}

bool Ntag::readchip(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data, byte data_size)
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

byte Ntag::writechip(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data, byte data_size)
{
    if(data_size>NTAG_BLOCK_SIZE || !writeMemAddress(bt, memBlockAddress)){
        return 0;
    }
    for (int i=0; i<data_size; i++)
    {
        if(HWire.write(p_data[i])!=1){
            break;
        }
    }
    if(!end_transmission()){
        return 0;
    }
    return data_size;
}

bool Ntag::read_register(REGISTER_NR regAddr, byte& value)
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

bool Ntag::write_register(REGISTER_NR regAddr, byte mask, byte regdat)
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
