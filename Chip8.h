#ifndef CHIP8_H
#define CHIP8_H

class chip8 {
private:
	//Memory
	unsigned char memory[4096]; //Has 4K memory
	//Registers
	unsigned char V[16]; //15 8bit general registers, 16th one used for carry flag
	unsigned short I; //Index register
	unsigned short PC; //Program counter
	unsigned short stack[16]; //Stack remembers the location before a jump to the subroutine
	unsigned short sp; //Stack pointer - used to remember which level of the stack is used
	unsigned short opcode; //opcodes are all two bytes long
	//Timers
	unsigned char delay_timer; //Both of these timers count at 60 Hz
	unsigned char sound_timer; //When set to zero they both will count down
public:
	//Input
	unsigned char key[16]; //Chip 8 has an 16 key HEX based keyboard
	bool keyPressed;
	//Screen
	unsigned char screen[64*32]; //Has a black and white screen with a total of 2048(64*32) pixels
	//Draw flag
	bool drawFlag;
	//Sound flag(to beep)
	bool soundFlag;
	//Initialization and cycle emulate functions
	void initialize();
	void emulateCycle();
};

#endif
