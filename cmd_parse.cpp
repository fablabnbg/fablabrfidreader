#include "cmd_parse.h"

Cmd_parse::Cmd_parse(){
	reset();
}

byte Cmd_parse::parse_char(char c){
	switch (state){
		case SKIP_WS:
			if ((c==' '||c=='\t'||c=='\r'||c=='\n')){
				break;
			}
			cmd=c;
			state=ARG1;
			break;
		case ARG1:
			arg1=c;
			state=ARG2;
			break;
		case ARG2:
			arg2=c;
			state=EXPECT_WS;
			break;
		case EXPECT_WS:
			if ((c==' '||c=='\t'||c=='\r'||c=='\n')){
				state=SKIP_WS;
				ready=1;
				return 1;
			}else{
				error=1;
				return -1;
			}
			break;
	}
	return 0

}

void reset(){
	ready=0;
	state=SKIP_WS;
	error=0;
}
