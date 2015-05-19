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
#include "cmd_parse.h"

#define RST_PIN		3
#define SS_PIN		9
#define BUZZ_PIN  5

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
FabNFC fabnfc(mfrc522);
Cmd_parse parser;

byte card_seen=false;
byte parsing


void setup() {
  pinMode(BUZZ_PIN,OUTPUT);
  Serial.begin(9600);		// Initialize serial communications with the PC
  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  Serial.println(F("Running, waiting for card..."));
}

void do_error(){
	Serial.println(F("Error"));
	parser.reset();
}

void do_command(){
	Serial.print(parser.cmd);
	Serial.print(parser.arg1);
	Serial.print(parser.arg2);
	parser.reset();
	switch (parser.cmd){
		case 'f':
			fabnfc.use_type=arg1;
			fabnfc.tag_type=arg2;
			break;
		case 'w':
			fabnfc.write();
			break;
		case 'r':
			Serial.print(fabnfc.isfablab);
			Serial.print(fabnfc.use_type);
			Serial.print(fabnfc.tag_type);
			break;
		default:
			Serial.println(F(" Err"));
			return;
	}
	Serial.println(F(" OK"));
}

void loop() {

  int res=fabnfc.identify();
  if (res==FabNFC::NO_CARD){
	  if (card_seen){
		  Serial.println(F("Card gone"));
	  }
  }else{
	  card_seen=true;
  }
  if (res==FabNFC::NO_MAGIC){
	  Serial.println(F("New foreign Card"));
  }
  if (res==FabNFC::OK){
	  Serial.println(F("New Fablab Card"));
  }

  while (Serial.available()){
	  byte r=parser.parse_char(Serial.read());
	  if (r!=0) break;
  }
  if (parser.error){
	  do_error();
  }
  if (parser.ready){
	  do_command();
  }

}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
