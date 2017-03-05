# arduino-ntag
Arduino library to interface through I²C with the NXP NTAG (NT3H1101 and NT3H1201)

# Hardware
## Tag
Altium Designer has been used to create a tag PCB with a [Class 6 type antenna](https://nxp.box.com/s/5wycjhfaglzkf77ggwtl).  
* [Schematic](https://drive.google.com/open?id=0B5_mAlpV8IjvOGVYaGR1VGNLQXM)
* [BoM](https://drive.google.com/open?id=0B5_mAlpV8IjvU2RXcXJ2NXU2TlU)

The [NXP-website](https://nxp-rfid.com/products/ntag/ntag-i2c-design-resources-full-access/) has a lot of useful resources for the NTAG-family of NFC-tags.  
For new designs, the [NT3H2111](http://www.nxp.com/products/identification-and-security/nfc-and-reader-ics/connected-tag-solutions/ntag-ic-plus-nfc-forum-type-2-tag-with-ic-interface-optimized-for-entry-level-nfc-applications:NT3H2111_2211) might be a better option.  It's cheaper, offers more functionality and should be compatible with the NT3H1101.

