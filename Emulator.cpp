#include "pch.h"
#include "Chip8.h"
#include <iostream>
#include <SDL.h>
#include <stdlib.h>
#include <time.h> 
#include <thread>
#include <chrono>

using namespace std;

//SDL Stuff
//Window parameters
const int SIZE_MULTIPLIER = 10;
const int SCREEN_WIDTH = 64 * SIZE_MULTIPLIER;
const int SCREEN_HEIGHT = 32 * SIZE_MULTIPLIER;
//Initialize SD:
bool init();
//Close SDL
void close();
//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The window renderer
SDL_Renderer* gRenderer = NULL;

chip8 myChip8;

int main(int argc, char *argv[])
{
	if (!init()) {
		cout << "Failed to initialize!\n" << endl;
	}
	else {
		//Initialize the Chip8 system and load the game into the memory
		myChip8.initialize();

		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//Emulation loop
		for (;;) {
			if (quit) break;

			//Emulate one cycle
			myChip8.emulateCycle();

			//If the draw flag is set, update the screen
			if (myChip8.drawFlag) {
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(gRenderer);

				//Draw each pixel
				for (int y = 0; y < 32; y++) {
					for (int x = 0; x < 64; x++) {
						if (myChip8.screen[(x + (y * 64))]) {
							SDL_Rect pixelRect = { x * SIZE_MULTIPLIER, y* SIZE_MULTIPLIER, (1)* SIZE_MULTIPLIER, (1)* SIZE_MULTIPLIER };
							SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
							SDL_RenderFillRect(gRenderer, &pixelRect);
						}
					}
				}

				//Update screen
				SDL_RenderPresent(gRenderer);
			}

			//Reset this before every key check
			myChip8.keyPressed = false;
			//Store key press state
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0) {
				//User requests quit
				if (e.type == SDL_QUIT) quit = true;
				//User presses a key
				if (e.type = SDL_KEYDOWN) {
					//Since a key has been pressed and we dont want to clear keys, we set this to true
					myChip8.keyPressed = true;
					switch (e.key.keysym.sym)
					{
					case SDLK_1:
						myChip8.key[0x1] = 1;
						break;
					case SDLK_2:
						myChip8.key[0x2] = 1;
						break;
					case SDLK_3:
						myChip8.key[0x3] = 1;
						break;
					case SDLK_4:
						myChip8.key[0xC] = 1;
						break;
					case SDLK_q:
						myChip8.key[0x4] = 1;
						break;
					case SDLK_w:
						myChip8.key[0x5] = 1;
						break;
					case SDLK_e:
						myChip8.key[0x6] = 1;
						break;
					case SDLK_r:
						myChip8.key[0xD] = 1;
						break;
					case SDLK_a:
						myChip8.key[0x7] = 1;
						break;
					case SDLK_s:
						myChip8.key[0x8] = 1;
						break;
					case SDLK_d:
						myChip8.key[0x9] = 1;
						break;
					case SDLK_f:
						myChip8.key[0xE] = 1;
						break;
					case SDLK_z:
						myChip8.key[0xA] = 1;
						break;
					case SDLK_x:
						myChip8.key[0x0] = 1;
						break;
					case SDLK_c:
						myChip8.key[0xB] = 1;
						break;
					case SDLK_v:
						myChip8.key[0xF] = 1;
						break;
					default:
						break;
					}
				}
			}
			//Clear all the keys only if none of them are pressed
			if (!myChip8.keyPressed) {
				for (int i = 0; i <= 15; i++) myChip8.key[i] = 0;
			}
			this_thread::sleep_for(chrono::microseconds(16666)); //We are using this to stabilize the clock speed of the CHIP8
			//CHIP8 clock speed is 60Hz, so it should have around 60opcodes per second, waiting for 16666 microseconds is the closest we can get to emulating this
		}
	}
	return 0;
}

bool init() {
	//Init success flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	else {
		//Create SDL Window
		gWindow = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
			success = false;
		}
		else {
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
				success = false;
			}
			else {
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
			}
		}
	}

	return success;
}

void close() {
	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gRenderer = NULL;
	gWindow = NULL;

	//Quit SDL
	SDL_Quit();
}