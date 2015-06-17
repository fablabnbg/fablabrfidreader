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
#include "MFRC522.h"
#include "fabnfc.h"
#include "cmd_parse.h"

#define RST_PIN		3
#define SS_PIN		9
#define BUZZ_PIN  5

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
FabNFC fabnfc(mfrc522);
Cmd_parse parser;

byte card_seen=false;
byte parsing;


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
	Serial.print(static_cast<char>(parser.cmd));
	Serial.print(static_cast<char>(parser.arg1));
	Serial.print(static_cast<char>(parser.arg2));
	Serial.print(' ');
	parser.reset();
	switch (parser.cmd){
		case 'f':
			fabnfc.use_type=parser.arg1;
			fabnfc.tag_type=parser.arg2;
			break;
		case 'w':
			fabnfc.write();
			break;
		case 'r':
			dump_byte_array(fabnfc.uid,sizeof(fabnfc.uid));
			Serial.print(':');
			Serial.print(fabnfc.isfablab);
			Serial.print(':');
			Serial.print(static_cast<char>(fabnfc.use_type));
			Serial.print(':');
			Serial.print(static_cast<char>(fabnfc.tag_type));
			break;
		default:
			Serial.println(F(" Err"));
			return;
	}
	Serial.println(F(" OK"));
}

void loop() {

	byte res=fabnfc.identify();
	if (res==FabNFC::NO_CARD){
		if (card_seen){
			Serial.println(F("Card gone"));
			card_seen=false;
		}
	}else{
		card_seen=true;
	}
	switch (res){
		case FabNFC::NO_MAGIC:
			Serial.print(F("New foreign Card:"));
			dump_byte_array(fabnfc.uid,sizeof(fabnfc.uid));
			Serial.println("");
			break;
		case FabNFC::OK:
			Serial.print(F("New Fablab Card:"));
			dump_byte_array(fabnfc.uid,sizeof(fabnfc.uid));
			Serial.println("");
			break;
		case FabNFC::UNSUPPORTED_CHIP:
			Serial.print(F("Unsupported chip:"));
			dump_byte_array(fabnfc.uid,sizeof(fabnfc.uid));
			Serial.println("");
			break;
		case FabNFC::UNSUPPORTED_CARD:
			Serial.print(F("Unsupported card:"));
			dump_byte_array(fabnfc.uid,sizeof(fabnfc.uid));
			Serial.println("");
			break;
		case FabNFC::NO_CARD:
		case FabNFC::SAME_CARD:
			break;
		default:
			Serial.print(F("Unknown reply to identify: "));
			Serial.println(res);
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
