/*
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 *
 * Pin layout used:
 * -------------------------------------
 * Signal      MFRC522      Arduino    
 * -------------------------------------
 * RST/Reset   RST          3          
 * SPI SS      SDA(SS)      9          
 * SPI MOSI    MOSI         11 / ICSP-4 
 * SPI MISO    MISO         12 / ICSP-1 
 * SPI SCK     SCK          13 / ICSP-3 
 */

#include <SPI.h>
#include <MFRC522.h>
#include "fabnfc.h"

#define RST_PIN		3
#define SS_PIN		9
#define BUZZ_PIN  5

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
FabNFC fabnfc(mfrc522);

void setup() {
  pinMode(BUZZ_PIN,OUTPUT);
  Serial.begin(9600);		// Initialize serial communications with the PC
  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  Serial.println(F("Running, waiting for card..."));
}

void loop() {

  int res=fabnfc.identify();
  if (!(FabNFC::NO_MAGIC==3 || FabNFC::OK==0)) return;

  if (res==FabNFC::NO_MAGIC) { // magic number 0xFA 0xB1 not present
    fabnfc.write();
    return;
  }

  Serial.print(F("UID:"));
  dump_byte_array(fabnfc.uid,7);
  Serial.println();

  if(fabnfc.pw_needed()){
    Serial.println(fabnfc.pw_auth());
  }

  // write stuffs
  fabnfc.use_type=FabNFC::ACCESS;
  fabnfc.tag_type=FabNFC::STICKER;
  

  fabnfc.data[0]=0x00;
  fabnfc.data[1]=0x00;
  fabnfc.data[2]=0x01;
  fabnfc.data[3]=0x01;

  fabnfc.datasize=4;
  
  fabnfc.write();

  //fabnfc.pw_set();

  

  digitalWrite(BUZZ_PIN,HIGH);
  delay(250);
  digitalWrite(BUZZ_PIN,LOW);

}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
