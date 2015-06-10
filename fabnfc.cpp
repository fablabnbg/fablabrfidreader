#include "fabnfc.h"

FabNFC::FabNFC(MFRC522& NFC):nfc(NFC),isfablab(false),datasize(0),has_card(false){
}

byte FabNFC::identify(){
	byte buffer[18];
	byte size=18;

	//check if card is still there
	if (nfc.MIFARE_Read(4,buffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
	}
	if (!has_card){
		memset(uid,0,sizeof(uid));
		if (! nfc.PICC_IsNewCardPresent()){
			return NO_CARD;
		}
		has_card=true;
		nfc.PICC_ReadCardSerial();
	}

	// is card's UID 7 Byte?
	if (nfc.uid.size!=7){
		isfablab=false;
		return UNSUPPORTED_CARD;
	}

	// is this card the same as last card?
	if (memcmp(uid,nfc.uid.uidByte,7)==0){
		return SAME_CARD;
	}

	// copy uid from card driver to local property
	for(byte i=0;i<7;i++){
		uid[i]=nfc.uid.uidByte[i];
	}

	// check magic bytes for fablab card
	if (nfc.MIFARE_Read(4,buffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
		return NO_CARD;
	}
	if (buffer[0]!=0xfa || buffer[1]!=0xb1){
		isfablab=false;
		return NO_MAGIC;
	}
	use_type=buffer[2];
	tag_type=buffer[3];

	// read CC (Capability Container) to identify chip
	if (nfc.MIFARE_Read(3,buffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
		return NO_CARD;
	}
	chip=buffer[2];

	// check if chip is supported
	if(chip!=0x6d) {
		return UNSUPPORTED_CHIP;
	}
	offset_confpages=0xe3;

	isfablab=true;
	return OK;
}

void FabNFC::read(){
	if (!has_card){
		return;
	}
	byte buffer[18];
	byte size=sizeof(buffer);
	if (nfc.MIFARE_Read(5,buffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
		return;
	}
	for(byte i=0;i<16;i++){
		data[i]=buffer[i];
	}
}

byte FabNFC::write(){
	if (!has_card){
		return 2;
	}
	if(datasize>=100) return 1; 
	isfablab=true;
	byte buffer[4];
	buffer[0]=0xfa;
	buffer[1]=0xb1;
	buffer[2]=use_type;
	buffer[3]=tag_type;
	if (nfc.MIFARE_Ultralight_Write(4,buffer,sizeof(buffer))!=MFRC522::STATUS_OK){
		has_card=false;
		return 2;
	}

	byte normsize=(datasize+4)>>2; // round to next multiple of 4 and divide by 4
	data[datasize]=0;

	for(byte block=0;block<normsize;block++){
		for(byte i=0;i<4;i++){
			buffer[i]=data[block*4+i];
		}
		if(nfc.MIFARE_Ultralight_Write(5+block,buffer,sizeof(buffer))!=MFRC522::STATUS_OK){
			has_card=false;
			return 2;
		}
	}
	return 0;
}

void FabNFC::halt(){
	nfc.PICC_HaltA();
}

byte FabNFC::pw_auth(){

	byte buffer[]={
		0x1b, // command for NTAG21x to auth
		0xfa,0xb1,0xab,0xff	// password to send
	};

	return nfc.PCD_MIFARE_Transceive(buffer,sizeof(buffer),false)==2;
	// 2 = maybe OK?
	// 4 = timeout/wrong pw

}

void FabNFC::pw_set(){
	// password
	byte buffer[]={
		0xfa,0xb1,0xab,0xff	// password to set
	};
	if (nfc.MIFARE_Ultralight_Write(offset_confpages+2,buffer,sizeof(buffer))!=MFRC522::STATUS_OK){
		has_card=false;
		return;
	}

	//set AUTH0 and ACCESS
	byte readbuffer[18];
	byte size=sizeof(readbuffer);
	if (nfc.MIFARE_Read(offset_confpages,readbuffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
		return;
	}
	readbuffer[3]=0;
	readbuffer[4]=0;
	if (nfc.MIFARE_Ultralight_Write(offset_confpages,readbuffer,4)!=MFRC522::STATUS_OK){
		has_card=false;
		return;
	}
	if (nfc.MIFARE_Ultralight_Write(offset_confpages+1,readbuffer+4,4)!=MFRC522::STATUS_OK){
		has_card=false;
		return;
	}
}

byte FabNFC::pw_needed(){
	//read AUTH0
	byte readbuffer[18];
	byte size=sizeof(readbuffer);
	if (nfc.MIFARE_Read(offset_confpages,readbuffer,&size)!=MFRC522::STATUS_OK){
		has_card=false;
		return 0;
	}
	if(readbuffer[3]<=30){
		return 1;
	}
	else {
		return 0;
	}
}
