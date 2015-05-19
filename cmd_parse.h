#ifndef CMD_PARSE_H
#define CMD_PARSE_H

struct Cmd_parse{
	enum {
		SKIP_WS,
		CMD,
		ARG1,
		ARG2,
		EXPECT_WS
	}
	Cmd_parse();

	byte parse_char(char c);
	void reset();
	byte ready;
	byte error;
	byte state;
	byte cmd;
	byte arg1;
	byte arg2;
};

#endif
