//Memory map:
//0x000 - 0x1FF - Chip 8 interpreter(Since this is an emulator, it containts teh font set)
//0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
//0x200 - 0xFFF - Program ROM and work RAM

//Used documentation about CHIP8 - https://en.wikipedia.org/wiki/CHIP-8

#include "pch.h"
#include "Chip8.h"
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8::initialize() { //Initialize registers and memory once
	PC = 0x200; //Program counter starts at 0x200
	opcode = 0; //Reset current opcode
	I = 0; //Reset index register
	sp = 0; //Reset stack pointer

	//Clear display
	for (int i = 0; i < 64 * 32; i++) screen[i] = 0;

	//Clear stack
	for (int i = 0; i < 16; i++) stack[i] = 0;

	//clear registers V0-VF
	for (int i = 0; i < 16; i++) V[i] = 0;

	//clear memory
	for (int i = 0; i < 4096; i++) memory[i] = 0;

	//Load fontset
	for (int i = 0; i < 80; i++) memory[i] = chip8_fontset[i];

	//Reset timers
	delay_timer = 0;
	sound_timer = 0;

	//Ask for game to run
	cout << "What game would you like to play? ";
	char gameName[10]; //At maximum the files name will be 10 characters
	cin >> gameName;
	for (int i = 0; i < 10; i++) gameName[i] = toupper(gameName[i]);

	//Open file
	FILE * gameFile = fopen(gameName, "rb");
	//Check if it was successfull
	if (gameFile == NULL) {
		cout << "File error" << endl;
	}

	//Get file size
	fseek(gameFile, 0, SEEK_END);
	long fileSize = ftell(gameFile);
	rewind(gameFile);

	//Allocate memory to containt the whole file in the buffer
	char * buffer = (char*)malloc(sizeof(char) * fileSize);
	//Check if it was successfull
	if (buffer == NULL) {
		cout << "Memory error" << endl;
	}

	//Copy file into the buffer
	fread(buffer, 1, fileSize, gameFile);

	//Copy buffer to memory
	if ((4096 - 512) > fileSize) {
		for (int i = 0; i < fileSize; ++i) memory[i + 512] = buffer[i];
	}

	//Close filer, free buffer
	fclose(gameFile);
	free(buffer);

	//Reset input
	for (int i = 0; i < 16; i++) key[i] = 0;
}

void chip8::emulateCycle() {
	//Fetch opcode
	opcode = memory[PC] << 8 | memory[PC + 1];

	//Decode opcode
	switch (opcode & 0xF000) //Using this switch and bitwise operations we determine what each opcode has to do
	{						 //Must remember that after almost every operation we need to increase the program counter by 2
	case 0x0000:
		switch (opcode & 0x000F) {
		case 0x0000: //Clears the screen
			for (int i = 0; i < 64 * 32; i++) screen[i] = 0;
			drawFlag = true;
			PC += 2;
			break;
		case 0x000E: //Returns from a subroutine
			--sp;
			PC = stack[sp];
			PC += 2;
			break;
		default: //On a default case the opcode is something unrecognized so we do tell that to the console for debugging reasons
			cout << "Unknown opcode: 0x" << opcode << endl;
			break;
		}
		break;
	case 0x1000: //Jumps to specified address
		PC = opcode & 0x0FFF;
		break;
	case 0x2000: //Calls subroutine at specified address
		stack[sp] = PC;
		++sp;
		PC = opcode & 0x0FFF;
		break;
	case 0x3000: //Skips the next instruction if specified register equals the given value
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) PC += 4;
		else PC += 2;
		break;
	case 0x4000: //Skips the next instruction if the specified register doesn't equal the given value
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) PC += 4;
		else PC += 2;
		break;
	case 0x5000: //Skips the next instruction if two given registers are equal
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) PC += 4;
		else PC += 2;
		break;
	case 0x6000: //Sets specified register to given value
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		PC += 2;
		break;
	case 0x7000: //Ads a specified value to a specified register
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		PC += 2;
		break;
	case 0x8000: //All thge opcodes that start with 8
		switch (opcode & 0x000F)
		{
		case 0x0000: //Sets the specified register to the value of a different register
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0001: //Sets specified register to a bitwise OR of the same and an another register
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0002: //Sets specified register to a bitwise AND of the same and an another register
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0003: //Sets specified register to a bitwise XOR of the same and an another register
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0004: //Ads two specified registers
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) V[0xF] = 1;//carry
			else V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0005: //Subtracts to specified registers
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) V[0xF] = 0; //borrow
			else V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			PC += 2;
			break;
		case 0x0006: //Stores the least significant bit of one register into the other one and shifts the first register right by 1
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			PC += 2;
			break;
		case 0x0007: //Sets one specified register to another one subtracted by the first one
			if ( V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) V[0xF] = 0; //borrow
			else V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			PC += 2;
			break;
		case 0x000E: //Stores the most significant bit of one register into the other one and shifts the first register left by 1
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			PC += 2;
			break;
		default: //On a default case the opcode is something unrecognized so we do tell that to the console for debugging reasons
			cout << "Unknown opcode: 0x" << opcode << endl;
			break;
		}
		break;
	case 0x9000: //Skips the next instruction if two registers don't equal each other
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) PC += 4;
		else PC += 2;
		break;
	case 0xA000: //Sets I to the specified address
		I = opcode & 0x0FFF;
		PC += 2;
		break;
	case 0xB000: //Jumps to the specified address plus V0
		PC = (opcode & 0x0FFF) + V[0x0];
		break;
	case 0xC000: //Sets the specified register to a bitwise AND on a random number and a given number (0-255)
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		PC += 2;
		break;
	case 0xD000: //Draws a sprite at a given coordinate and sets the draw flag to true
		{
		unsigned short startX = V[(opcode & 0x0F00) >> 8];
		unsigned short startY = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int y = 0; y < height; y++) {
			pixel = memory[I + y];
			for (int x = 0; x < 8; x++) {
				if ((pixel & (0x80 >> x)) != 0) {
					if (screen[(startX + x + ((startY + y) * 64))] == 1) V[0xF] = 1;
					screen[startX + x + ((startY + y) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		PC += 2;
		}
		break;
	case 0xE000: //All opcodes that start with E
		switch (opcode & 0x00FF)
		{
		case 0x009E: //Skips the next instruction if the key stored in given register is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] != 0) PC += 4;
			else PC += 2;
			break;
		case 0x00A1: //Skips the next instruction if the key stored in given register is not pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0) PC += 4;
			else PC += 2;
			break;
		default: //On a default case the opcode is something unrecognized so we do tell that to the console for debugging reasons
			cout << "Unknown opcode: 0x" << opcode << endl;
			break;
		}
		break;
	case 0xF000: //All opcodes that start with F
		switch (opcode & 0x00FF)
		{
		case 0x0007: //Sets the specified register to the value of the delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			PC += 2;
			break;
		case 0x000A: //A key press is awaited and then stored in a specified register
			{
			bool keyPress = false;

			for (int i = 0; i < 16; i++) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			//If we didnt get a keypress skip and try again
			if (!keyPress) return;

			PC += 2;
			}
			break;
		case 0x0015: //Sets the delay timer to a specified registers value
			delay_timer = V[(opcode & 0x0F00) >> 8];
			PC += 2;
			break;
		case 0x0018: //Sets the sound timer to a specified registers value
			sound_timer = V[(opcode & 0x0F00) >> 8];
			PC += 2;
			break;
		case 0x001E: //Adds specified register to I (VF is set to 1 when there is a range overflow and not when there isn't)
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) V[0xF] = 1;
			else V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			PC += 2;
			break;
		case 0x0029: //Sets I the location of the sprite for the character in the given register
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			PC += 2;
			break;
		case 0x0033: //Stores the binary coded decimal representation of given register into memory
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			PC += 2;
			break;
		case 0x0055: //Stores V0 to given register in memory starting at address I
			for (int i = 0x0; i <= ((opcode & 0x0F00) >> 8); i++) memory[I + i] = V[i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			PC += 2;
			break;
		case 0x0065: //Fills V0 to given register with calues from memory starting at address I
			for (int i = 0x0; i <= ((opcode & 0x0F00) >> 8); i++) V[i] = memory[I + i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			PC += 2;
			break;
		default: //On a default case the opcode is something unrecognized so we do tell that to the console for debugging reasons
			cout << "Unknown opcode: 0x" << opcode << endl;
			break;
		}
		break;
	default: //On a default case the opcode is something unrecognized so we do tell that to the console for debugging reasons
		cout << "Unknown opcode: 0x" << opcode << endl;
		break;
	}

	//Update timers
	if (delay_timer > 0) --delay_timer;

	if (sound_timer > 0) {
		cout << '\a'; //Beeps if it needs to
		--sound_timer;
	}
}
