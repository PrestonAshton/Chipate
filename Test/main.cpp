#define SDL_MAIN_HANDLED

#include <Propitious/Common.hpp>

#include "Chip8.hpp"

#include <SDL/SDL.h>

using namespace Propitious;

int main(int argc, char** argv)
{
	CPU::Chip8 cpu;
	CPU::init(cpu);

	CPU::loadGame("tetris.ch8", cpu, 0x200);

	SDL_Window* window = SDL_CreateWindow("Test Thing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);

	for (;;)
	{
		CPU::cycle(cpu);
		for (u16 i = 0; i < 2048; i++)
		{
			u32 *pixels = (u32*)surface->pixels;
			pixels[i] = cpu.graphics[i];
		}
	}
	return 1;
}