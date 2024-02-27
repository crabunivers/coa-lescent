#include <stdio.h>

unsigned char flag_register = 0x00; // this is equivalent to this new implementation below
// MSb to LSb: CarryFlag|ZeroFlag|OverflowFlag|SignFlag

bool carryFlag;
bool zeroFlag;
bool overflowFlag;
bool signFlag;

unsigned int ACC; 
// The 15 MSb should NOT be used, it will also not be considered and normalized to a 17-bit number before any operations

void normalizeACC(){
	ACC = ACC & 0x0001FFFF; // make ACC a 16+1 bit register, the rest will be standardized values
	return;
}

enum ControlSignals{
	NOP = 0x00,
	
	ADD = 0x01,
	SUB,
	MUL,
	
	AND = 0x04,
	OR,
	NOT,
	XOR,
	
	LSHL = 0x09,
	LSHR = 0x08
};

unsigned char twosComp(unsigned char operand){
	unsigned char negative_operand = !operand;
	negative_operand++;
	return negative_operand;
}

void setFlags(unsigned char operand1, unsigned char operand2, unsigned char control_signal){
	normalizeACC();
	switch(control_signal){
		case NOP:
		case NOT:
			break;
		case SUB:
		case ADD:
			flagcheck = ACC & 0x00000100;
			if(flagcheck == 0x00000100) carryFlag = TRUE;
			else carryFlag = FALSE;

			flagcheck = ACC & 0x000000FF;
			if (flagcheck == 0) zeroFlag = TRUE;
			else zeroFlagCheck = FALSE;

			if (operand1 >= 0x80 && operand2 >= 0x80 && flagcheck <= 0x7F) overflowFlag = TRUE;
			else if (operand1 <= 0x7F && operand2 <= 0x7F && flagcheck >= 0x80) overflowFlag = TRUE;
			else overflowFlag = FALSE;

			flagcheck = ACC & 0x00000080;
			if (flagcheck == 0x00000080) signFlag = TRUE;
			else signFlag = FALSE;
			break;
		case MUL: // the setting of flags here is based on the IMUL 8086 instruction
			flagcheck = ACC & 0x0000FFFF;
			if (flagcheck >= 0x0000FF80 && flagcheck <= 0x0000007F){
				carryFlag = FALSE;
				overflowFlag = FALSE;
			}else{
				carryFlag = TRUE;
				overflowFlag = TRUE;
			}
		case AND:
		case OR:
		case XOR:
			carryFlag = FALSE;

			flagcheck = ACC & 0x000000FF;
			if (flagcheck == 0) zeroFlag = TRUE;
			else zeroFlagCheck = FALSE;

			overflowFlag = FALSE;

			flagcheck = ACC & 0x00000080;
			if (flagcheck == 0x00000080) signFlag = TRUE;
			else signFlag = FALSE;
			break;
		case LSHL:
		case LSHR:
			flagcheck = ACC & 0x00000100;
			if(flagcheck == 0x00000100) carryFlag = TRUE;
			else carryFlag = FALSE;

			flagcheck = ACC & 0x000000FF;
			if (flagcheck == 0) zeroFlag = TRUE;
			else zeroFlag = FALSE;

			if(operand1 >= 0x80 && flagcheck >= 0x80) overflowFlag = FALSE;
			else if(operand1 <= 0x7F && flagcheck <= 0x7F) overflowFlag = FALSE;
			else overflowFlag = TRUE;
			
			flagcheck = ACC & 0x00000080;
			if(flagcheck == 0x00000080) signFlag = TRUE;
			else signFlag = FALSE;
			break;
		default:
	}
	return;
}

void printBin (unsigned int data, unsigned char data_width){
	unsigned char n = data_width;
	char data_bin_str[n];
	while(n > 0){
		n--;
		data_bin_str[n] = data % 2;
		data >>= 1;
	}
	printf ("%s", data_bin_str);
	return;
}

void throwControlSignalError (unsigned char control_signal){
	printf("0x%08x is not a valid control signal!", control_signal);
	return;
}

void ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signal){
	unsigned char temp_op1 = operand1, temp_op2 = operand2;
	
	switch(control_signal){
		case NOP: 
			break;
		//=====ARITHMETIC=====//
		case SUB: 
			temp_op2 = twosComp(temp_op2);
		case ADD:
			ACC = temp_op1 + temp_op2;
			break;
		case MUL:
			
			break;
		//=====LOGIC=====//
		case AND: 
			ACC = temp_op1 & temp_op2;
			break;
		case OR:
			ACC = temp_op1 | temp_op2;
			break;
		case NOT: 
			ACC = !temp_op1;
			break;
		case XOR: 
			ACC = temp_op1 ^ temp_op2;
			break;
		case LSHL:
			ACC = temp_op1;
			while(temp_op2 > 0){
				ACC <<= 1;
				temp_op2--;
			}
			temp_op1 = operand1;
			break;
		case LSHR: 
			ACC = temp_op1;
			while(temp_op2 > 0){
				if(temp_op2 == 1){
					temp_op1 = ACC % 2;
				}
				ACC >>= 1;
				if(temp_op2 == 1){
					ACC = temp_op1 * 0x0100;
				}
				temp_op2--;
			}
			temp_op1 = operand1;
			break;
		default:
			throwControlSignalError(control_signals);
	}
	setFlags(temp_op1, temp_op2, control_signal);
}

int main(){
	//=====INIT=====//
	unsigned char register1, register2, control_input;
	
	//=====NOP-TEST=====//
	register1 = 0x7F;
	register2 = 0x80;
	control_input = NOP;
	ALU(register1, register2, control_input);
	
	//=====ADD-TEST=====//
	register1 = 0x18; // 0001 1000 = 24
	register2 = 0x12; // 0001 0010 = 18
	control_input = ADD; // expected: 42
	ALU(register1, register2, control_input);
	
	register1 = 0xFC; // 1111 1100 = -4
	register2 = 0xA9; // 1010 1001 = -87
	control_input = ADD; // expected: -91
	ALU(register1, register2, control_input);
	
	register1 = 0x19; // 0001 1101 = 29
	register2 = 0xA9; // 1010 1001 = -87
	control_input = ADD; // expected: -58
	ALU(register1, register2, control_input);
	
	//=====SUB-TEST=====//
	register1 = 0xFC; // 1111 1100 = -4
	register2 = 0x79; // 0111 1001 = 121
	control_input = SUB; // expected: -125
	ALU(register1, register2, control_input);
	
	register1 = 0xFC; // 1111 1100 = -4
	register2 = 0x79; // 0111 1001 = 121
	control_input = SUB; // expected: -125
	ALU(register1, register2, control_input);
	
	register1 = 0xFC; // 1111 1100 = -4
	register2 = 0x79; // 0111 1001 = 121
	control_input = SUB; // expected: -125
	ALU(register1, register2, control_input);
	
	//=====MUL-TEST=====//
	register1 = 0xFC; // 1111 1100 = -4
	register2 = 0x79; // 0111 1001 = 121
	control_input = MUL; // expected: -125
	ALU(register1, register2, control_input);
	
	//=====AND-TEST=====//
	register1 = 0x18; // 0001 1000
	register2 = 0x37; // 0011 0111
	control_input = AND; // expected: 0001 0000
	ALU(register1, register2, control_input);
	
	//=====OR-TEST=====//
	register1 = 0x18; // 0001 1000
	register2 = 0x37; // 0011 0111
	control_input = OR; // expected: 0011 1111
	ALU(register1, register2, control_input);
	
	//=====NOT-TEST=====//
	register1 = 0x18; // 0001 1000
	control_input = NOT; // expected: 1110 0111
	ALU(register1, register2, control_input);
	
	//=====XOR-TEST=====//
	register1 = 0x19; // 0001 1001
	register2 = 0x37; // 0011 0111
	control_input = XOR; // expected: 0010 1110
	ALU(register1, register2, control_input);
	
	//=====LSHL-TEST=====//
	register1 = 0x99; // 1001 1001
	register2 = 2;
	control_input = LSHL; // expected: 0110 0100
	ALU(register1, register2, control_input);
	
	//=====RSHL-TEST=====//
	register1 = 0x99; // 1001 1001
	register2 = 2;
	control_input = LSHR; // expected: 0010 0110
	ALU(register1, register2, control_input);
	
	return 0;
}
