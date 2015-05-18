#include "fabnfc.h"

FabNFC::FabNFC(MFRC522& NFC):nfc(NFC),datasize(0){
}

byte FabNFC::identify(){
	if (!nfc.PICC_IsNewCardPresent()){
		return NO_CARD;
	}

	// is card's UID 7 Byte?
	nfc.PICC_ReadCardSerial();
	if (nfc.uid.size!=7){
		return UNSUPPORTED_CARD;
	}

	// is this card the same as last card?
	if (memcmp(uid,nfc.uid.uidByte,7)==0){
		return SAME_CARD;
	}

	byte buffer[18];
	byte size=18;

	// check magic bytes for fablab card
	nfc.MIFARE_Read(4,buffer,&size);
	if (buffer[0]!=0xfa || buffer[1]!=0xb1){
		return NO_MAGIC;
	}
	use_type=buffer[2];
	tag_type=buffer[3];

	// read CC (Capability Container) to identify chip
	nfc.MIFARE_Read(3,buffer,&size);
	chip=buffer[2];

	// check if chip is supported
	if(chip!=0x6d) {
		return UNSUPPORTED_CHIP;
	}
	offset_confpages=0xe3;

	for(byte i=0;i<7;i++){
		uid[i]=nfc.uid.uidByte[i];
	}
	return OK;
}

void FabNFC::read(){
	byte buffer[18];
	byte size=sizeof(buffer);
	nfc.MIFARE_Read(5,buffer,&size);
	for(byte i=0;i<16;i++){
		data[i]=buffer[i];
	}
}

byte FabNFC::write(){
	if(datasize>=100) return 1; 
	byte buffer[4];
	buffer[0]=0xfa;
	buffer[1]=0xb1;
	buffer[2]=use_type;
	buffer[3]=tag_type;
	nfc.MIFARE_Ultralight_Write(4,buffer,sizeof(buffer));

	byte normsize=(datasize+4)>>2; // round to next multiple of 4 and divide by 4
	data[datasize]=0;

	for(byte block=0;block<normsize;block++){
		for(byte i=0;i<4;i++){
			buffer[i]=data[block*4+i];
		}
		nfc.MIFARE_Ultralight_Write(5+block,buffer,sizeof(buffer));
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
	nfc.MIFARE_Ultralight_Write(offset_confpages+2,buffer,sizeof(buffer));

	//set AUTH0 and ACCESS
	byte readbuffer[18];
	byte size=sizeof(readbuffer);
	nfc.MIFARE_Read(offset_confpages,readbuffer,&size);
	readbuffer[3]=0;
	readbuffer[4]=0;
	nfc.MIFARE_Ultralight_Write(offset_confpages,readbuffer,4);
	nfc.MIFARE_Ultralight_Write(offset_confpages+1,readbuffer+4,4);
}

byte FabNFC::pw_needed(){
	//read AUTH0
	byte readbuffer[18];
	byte size=sizeof(readbuffer);
	nfc.MIFARE_Read(offset_confpages,readbuffer,&size);
	if(readbuffer[3]<=30){
		return 1;
	}
	else {
		return 0;
	}
}
