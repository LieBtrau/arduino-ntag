# arduino-ntag
Arduino library to interface with the NXP NTAG (NT3H1101 and NT3H1201)

# Hardware
## Tag
Altium Designer has been used to create a tag PCB with a [Class 6 type antenna](https://nxp.box.com/s/5wycjhfaglzkf77ggwtl).  
* [Schematic](https://drive.google.com/open?id=0B5_mAlpV8IjvOGVYaGR1VGNLQXM)
* [BoM](https://drive.google.com/open?id=0B5_mAlpV8IjvU2RXcXJ2NXU2TlU)

The [NXP-website](https://nxp-rfid.com/products/ntag/ntag-i2c-design-resources-full-access/) has a lot of useful resources for the NTAG-family of NFC-tags.  
For new designs, the [NT3H2111](http://www.nxp.com/products/identification-and-security/nfc-and-reader-ics/connected-tag-solutions/ntag-ic-plus-nfc-forum-type-2-tag-with-ic-interface-optimized-for-entry-level-nfc-applications:NT3H2111_2211) might be a better option.  It's cheaper, offers more functionality and should be compatible with the NT3H1101.

## Reader
* *[Elechouse](http://www.elechouse.com/elechouse/index.php?main_page=product_info&cPath=90_93&products_id=2242)*: very cheap, small reader, probably not the best range.
* EXPLORE-NFC, Farnell 2366201, uses PN512, €19.77: RaspberryPi add on
* M24LR-DISCOVERY, Farnell 2280018, 2 boards: reader + tag emulator
* http://www.mikroe.com/click/rfid/ MIKROE-1434: RS 862-4847 (€26), uses  CR95HF
* Adafruit NFC reader  

Connection of Elechouse reader to Arduino Uno:
* VCC=5V
* IRQ=2
* RSTPDN=3
* SDA=A4
* SCL=A5
* GND=GND

#  Firmware
## Existing libraries
* Elechouse: code doesn't work well, don't use
* Adafruit: works for Uno + Due, but no NDEF support
* Seeedstudio + Don: only Uno, has NDEF support:
 * https://don.github.io/slides/2014-09-21-makerfaire-nfc/#/20
 * https://github.com/Seeed-Studio/PN532	
 * https://github.com/don/Ndef

