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

bool Ntag::getSerialNumber(byte* sn){
    byte data[7];
    if(!read(0,data,7)){
        return false;
    }
    if(data[0]!=4){
        return false;
    }
    memcpy(sn, data,7);
    return true;
}


bool Ntag::read(byte mema, byte *p_data, byte data_size)
{
    if(data_size>NTAG_BLOCK_SIZE || !writeMemAddress(USERDATA, mema)){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,data_size)!=data_size){
	return false;
    }
    for (int i=0; i<data_size && HWire.available(); i++)
    {
        p_data[i] = HWire.read();
    }
    return true;
}

byte Ntag::write(byte mema, byte *p_data, byte data_size)
{
    if(data_size>NTAG_BLOCK_SIZE || !writeMemAddress(USERDATA, mema)){
        return 0;
    }
    HWire.beginTransmission(_i2c_address);
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

bool Ntag::read_register(byte regAddr, byte& value)
{
    value=0;
    bool bRetVal=true;
    if(regAddr>7 || !writeMemAddress(REGISTER, 0xFE)){
        return false;
    }
    if(HWire.write(regAddr)!=1){
        bRetVal=false;
    }
    if(!HWire.endTransmission()){
	return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,(byte)1)!=1){
	return false;
    }
    value=HWire.read();
    return bRetVal;
}

bool Ntag::write_register(byte regAddr, byte mask, byte regdat)
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
    bool bRetVal =  HWire.write(addr)==1;
    if(dt==USERDATA){
        bRetVal &= end_transmission();
    }
    return bRetVal;
}

bool Ntag::end_transmission(void)
{
    return HWire.endTransmission()==0;
    //I2C_LOCKED must be either reset to 0b at the end of the I2C sequence or wait until the end of the watch dog timer.
}

bool Ntag::isAddressValid(BLOCK_TYPE type, byte address){
    switch(type){
    case USERDATA:
        switch (_dt) {
        case NTAG_I2C_1K:
            if((address > 0x3A && address < 0xF8) || address > 0xFB){
                return false;
            }
            break;
        case NTAG_I2C_2K:
            if((address > 0x7A && address < 0xF8) || address > 0xFB){
                return false;
            }
            break;
        default:
            return false;
        }
        break;
    case REGISTER:
        if(address!=0xFE){
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}
