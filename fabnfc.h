#ifndef FABNFC_H
#define FABNFC_H
#include <MFRC522.h>

class FabNFC{
  public:
    enum Use_Type{
      ACCESS='A',
      INVENTORY='I'
    };
    enum Tag_Type{
      CARD='C',
      KEY='K',
      STICKER='S',
      IMPLANT='I'
    };
	enum IDENT_RESULT{
		OK=0,
		NO_CARD=1,
		UNSUPPORTED_CARD=2,
		NO_MAGIC=3,
		UNSUPPORTED_CHIP=4,
		SAME_CARD=0xFF
	};

    FabNFC(MFRC522& NFC);
    byte identify();
    void read();
    byte write();
    void halt();
    byte pw_auth();
    void pw_set();
    byte pw_needed();
    byte uid[7];
	byte isfablab;
    byte use_type;
    byte tag_type;
    byte locks[2];
    byte data[100];
    byte datasize;
    byte chip;
    byte offset_confpages;
  private:
    MFRC522& nfc;
};

#endif
