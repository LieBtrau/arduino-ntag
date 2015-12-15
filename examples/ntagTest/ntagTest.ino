#include "Arduino.h"
#define HARDI2C
#include <Wire.h>
#include "ntag.h"

Ntag ntag(Ntag::NTAG_I2C_1K);

void setup(){
    Serial.begin(115200);
    Serial.println("start");
    if(!ntag.begin()){
        Serial.println("Can't find ntag");
    }
    testSn();
    testUserMem();
    testRegisterAccess();
}

void loop(){

}

void testRegisterAccess(){
    byte data;
    Serial.println(ntag.readRegister(Ntag::NC_REG,data));
    Serial.println(data,HEX);
    Serial.println(ntag.writeRegister(Ntag::NC_REG,0x0C,0x0A));
    Serial.println(ntag.readRegister(Ntag::NC_REG,data));
    Serial.println(data,HEX);
}

void testSn(){
    byte sn[7];
    Serial.println();
    Serial.print("Serial number of the tag is: ");
    if(ntag.getSerialNumber(sn)){
        for(byte i=0;i<7;i++){
            Serial.print(sn[i], HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
}

void testUserMem(){
    byte eepromdata[2*16];
    byte readeeprom[16];

    for(byte i=0;i<2*16;i++){
        eepromdata[i]=0x80 | i;
    }

    Serial.println("Writing block 1");
    if(!ntag.writeUserMem(1,eepromdata)){
        Serial.println("Write block 1 failed");
    }
    Serial.println("Writing block 2");
    if(!ntag.writeUserMem(2,eepromdata+16)){
        Serial.println("Write block 2 failed");
    }
    Serial.println("\nReading memory block 1");
    if(ntag.readUserMem(1,readeeprom,16)){
        for(int i=0;i<16;i++){
            Serial.print(readeeprom[i],HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    Serial.println("Reading memory block 2");
    if(ntag.readUserMem(2,readeeprom,16)){
        for(int i=0;i<16;i++){
            Serial.print(readeeprom[i],HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    Serial.println("Reading bytes 10 to 20: partly block 1, partly block 2");
    if(ntag.read(10,readeeprom,10)){
        for(int i=0;i<10;i++){
            Serial.print(readeeprom[i],HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    Serial.println("Writing byte 15 to 20: partly block 1, partly block 2");
    for(byte i=0;i<6;i++){
        eepromdata[i]=0x70 | i;
    }
    if(ntag.write(15,eepromdata,6)){
        Serial.println("Write success");
    }
    Serial.println("\nReading memory block 1");
    Serial.println("\nReading memory block 1");
    if(ntag.readUserMem(1,readeeprom,16)){
        for(int i=0;i<16;i++){
            Serial.print(readeeprom[i],HEX);
            Serial.print(" ");
        }
    }
    Serial.println();
    Serial.println("Reading memory block 2");
    if(ntag.readUserMem(2,readeeprom,16)){
        for(int i=0;i<16;i++){
            Serial.print(readeeprom[i],HEX);
            Serial.print(" ");
        }
    }
}
