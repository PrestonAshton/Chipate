#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <Propitious/Common.hpp>
#include <string>
#include <iomanip>
#include <Windows.h>

using namespace Propitious;

namespace CPU
{
	struct Chip8
	{
		u16 opcode;
		u8 memory[4096];
		u8 v[16];
		u16 i;
		u16 programCounter;
		u8 graphics[2048];
		u8 delayTimer;
		u8 soundTimer;
		u16 stack[16];
		u8 stackPointer;
		u8 key[16];
	};

	void init(Chip8& cpu)
	{
		cpu.opcode = 0;
		cpu.i = 0;
		cpu.stackPointer = 0;

		for (u8 i = 0; i < 16; i++)
		{
			cpu.stack[i] = 0;
		}


		for (u8 i = 0; i < 16; i++)
		{
			cpu.v[i] = 0;
		}

		for (u16 i = 0; i < 2048; i++)
		{
			cpu.graphics[i] = 0;
		}

		for (u16 i = 0; i < 4096; i++)
		{
			cpu.memory[i] = 0;
		}

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

		for (u8 i = 0; i < 80; i++)
		{
			cpu.memory[i] = chip8_fontset[i];
		}
	}

	void loadGame(const std::string& filename, Chip8& cpu, u16 startPoint)
	{
		cpu.programCounter = startPoint;
		std::basic_ifstream<u8> file;

		i32 current = 0;
		file.open(filename.c_str(), std::ios::binary|std::ios::in);
		if (file.is_open())
		{
			file.seekg(0, std::ios::end);
			usize size = file.tellg();
			file.seekg(0, std::ios::beg);
			file.read(&cpu.memory[startPoint], size);
		}
	}

	void cycle(Chip8& cpu)
	{
		//if (cpu.programCounter < 0x200)
		//	cpu.programCounter = 0x200;

		cpu.opcode = cpu.memory[cpu.programCounter];
		cpu.opcode = cpu.opcode << 8;
		cpu.opcode = cpu.opcode | cpu.memory[cpu.programCounter + 1];

		std::cout << "Info: Opcode - 0x" << std::hex << cpu.opcode << std::dec << " | PC - " << cpu.programCounter << std::endl;

		// Chip8 Opcodes are 2 bytes

		// 0x1000 =
		//  4    3    2    1 // nibbles
		// 0001 0000 0000 0000
		// To get first part...
		// 0x1000 >> 12 (3 * 4)

		// 0x0 or 0x1 etc...
		u8 nibble1 = (cpu.opcode & 0xF000) >> 12; //3 * 4 // Get the first  nibble with the bitwise and.
		u8 nibble2 = (cpu.opcode & 0x0F00) >> 8; // 2 * 4 // Get the second nibble with the bitwise and.
		u8 nibble3 = (cpu.opcode & 0x00F0) >> 4; // 1 * 4 // Get the third  nibble with the bitwise and.
		u8 nibble4 = (cpu.opcode & 0x000F) >> 0; // 0 * 4 // Get the fourth nibble with the bitwise and.

		u16 nibble23 =
			(nibble2 << 4) |
			(nibble3 << 0);

		u16 nibble234 =
			(nibble2 << 8) |
			(nibble3 << 4) |
			(nibble4 << 0);

		u16 nibble34 =
			(nibble3 << 4) |
			(nibble4 << 0);

		switch (nibble1)
		{
		case 0x0:
		{
			switch (nibble234)
			{
			case 0x0E0:
			{
				// 00E0 - CLS
				// Clear the screen.
				// TODO: this
				cpu.programCounter += 2;
				break;
			}
			case 0x0EE:
			{
				// 00EE - RET
				// Return from a subroutine.
				// Set program counter to the address at the top of the stack and subtract 1 from the stack pointer.
				cpu.programCounter = cpu.stack[--cpu.stackPointer];
				break;
			}
			default:
			{
				// 0nnn - SYS addr
				// Jump to a machine code routine at nnn.
				// Generally ignored by modern Chip8 stuff so, ignored...
				cpu.programCounter += 2;
				break;
			}
			} break;
		}
		case 0x1:
		{
			// 1nnn - JP addr
			// Jump to location nnn.
			// Set program counter to nnn.
			cpu.programCounter = nibble234;
			break;
		}
		case 0x2:
		{
			// 2nnn - CALL addr
			// Call subroutine at nnn.
			// Increment the stack pointer and put the program counter at the top of the stack, program counter set to nnn.
			cpu.stack[cpu.stackPointer] = cpu.programCounter;
			cpu.programCounter = nibble234;
			cpu.stackPointer++;
			break;
		}
		case 0x3:
		{
			// 3xkk - SE Vx, byte
			// Skip next instruction if Vx = kk.
			// Compare Vx to kk and if they are equal increment the program counter by 2.
			if (cpu.v[nibble2] == nibble34)
				cpu.programCounter += 4;
			else
				cpu.programCounter += 2;
			break;
		}
		case 0x4:
		{
			// 4xkk SNE Vx, byte
			// Skip next instruction if Vx != kk.
			// Compare Vx to kk and if they are not equal increment the program counter by 2.
			if (cpu.v[nibble2] != nibble34)
				cpu.programCounter += 4;
			else
				cpu.programCounter += 2;
			break;
		}
		case 0x5:
		{
			// 5xy0 - SE Vx, Vy
			// TODO: does the 0 *have* to be there is it common in most interps.
			// Skip next instruction if Vx = Vy.
			// Compare register Vx to Vy and if they are equal increment the program counter by 2.
			if (cpu.v[nibble2] == cpu.v[nibble3])
				cpu.programCounter += 4;
			else
				cpu.programCounter += 2;
			break;
		}
		case 0x6:
		{
			// 6xkk - LD Vx, byte
			// Set Vx = Vx + kk.
			// Put value kk into register Vx.
			cpu.v[nibble2] = nibble34;
			cpu.programCounter += 2;
			break;
		}
		case 0x7:
		{
			// 7xkk - ADD Vx, byte
			// Set Vx = Vx + kk.
			// Add the value kk to the value of register Vx then store the result in Vx.
			cpu.v[nibble2] = cpu.v[nibble2] + nibble34;
			cpu.programCounter += 2;
			break;
		}
		case 0x8:
		{
			switch (nibble4)
			{
			case 0x0:
			{
				// 8xy0 - LD Vx, Vy
				// Set Vx = Vy.
				// Store the value of register Vy into register Vx.
				cpu.v[nibble2] = cpu.v[nibble3];
				cpu.programCounter += 2;
				break;
			}
			case 0x1:
			{
				// 8xy1 - OR Vx, Vy
				// Set Vx = Vx OR Vy.
				// Performs a bitwise OR on the values of Vx and Vy and stores the value in Vx.
				cpu.v[nibble2] = cpu.v[nibble2] | cpu.v[nibble3];
				cpu.programCounter += 2;
				break;
			}
			case 0x2:
			{
				// 8xy2 - AND Vx, Vy
				// Set Vx = Vx AND Vy.
				// Performs a bitwise AND on the values of Vx and Vy and stores the value in Vx.
				cpu.v[nibble2] = cpu.v[nibble2] & cpu.v[nibble3];
				cpu.programCounter += 2;

				break;
			}
			case 0x3:
			{
				// 8xy3 - XOR Vx, Vy
				// Set Vx = Vx XOR Vy.
				// Performs a bitwise XOR on the values of Vx and Vy and stores the value in Vx.
				cpu.v[nibble2] = cpu.v[nibble2] ^ cpu.v[nibble3];
				cpu.programCounter += 2;
				break;
			}
			case 0x4:
			{
				// 8xy4 - ADD Vx, Vy
				// Set Vx = Vx + Vy, set VF = carry.
				// The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
				u16 result = cpu.v[nibble2] + cpu.v[nibble3];
				if (result > 255)
				{
					result = result - 255;
					cpu.v[0xF] = 1;
				}
				else
				{
					cpu.v[0xF] = 0;
				}
				cpu.v[nibble2] = result;
				cpu.programCounter += 2;
				break;
			}
			case 0x5:
			{
				// 8xy5 - SUB Vx, Vy
				// Set Vx = Vx - Vy, set VF = NOT borrow.
				// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
				if (cpu.v[nibble2] > cpu.v[nibble3])
					cpu.v[0xF] = 1;
				else
					cpu.v[0xF] = 0;

				cpu.v[nibble2] = cpu.v[nibble3] - cpu.v[nibble2];
				cpu.programCounter += 2;
				break;
			}
			case 0x6:
			{
				// 8xy6 - SHR Vx {, Vy}
				// Set Vx = Vx SHR 1.
				// If the least - significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
				if ((cpu.v[nibble2] % 2) == 1)
					cpu.v[0xF] = 1;
				else
					cpu.v[0xF] = 0;

				cpu.v[nibble2] = cpu.v[nibble2] / 2;
				cpu.programCounter += 2;
				break;
			}
			case 0x7:
			{
				// 8xy6 - SUBN Vx, Vy
				// Set Vx = Vy - Vx, set VF = NOT borrow.
				// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
				if (cpu.v[nibble3] > cpu.v[nibble2])
					cpu.v[0xF] = 1;
				else
					cpu.v[0xF] = 0;

				cpu.v[nibble2] = cpu.v[nibble2] - cpu.v[nibble3];
				cpu.programCounter += 2;
				break;
			}
			default:
			{
				// 8xyE - SHL Vx {, Vy}
				// Set Vx = Vx SHL 1.
				// If the most - significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
				if ((cpu.v[nibble2] % 2) == 1)
					cpu.v[0xF] = 1;
				else
					cpu.v[0xF] = 0;

				cpu.v[nibble2] = cpu.v[nibble2] * 2;
				cpu.programCounter += 2;
				break;
			}
			} break;
		}
		case 0x9:
		{
			// 9xy0 - SNE Vx, Vy
			// The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
			if (cpu.v[nibble2] != cpu.v[nibble3])
				cpu.programCounter += 2;
			break;
		}
		case 0xA:
		{
			// Annn - LD I, addr
			// Set I = nnn.
			// The value of register I is set to nnn.
			cpu.i = (u8)nibble234;
			cpu.programCounter += 2;
			break;
		}
		case 0xB:
		{
			// Bnnn - JP V0, addr
			// Jump to location nnn + V0.
			// The program counter is set to nnn plus the value of V0.
			cpu.programCounter = nibble234 + cpu.v[0];
			cpu.programCounter += 2;
			break;
		}
		case 0xC:
		{
			// Cxkk - RND Vx, byte
			// Set Vx = random byte AND kk.
			// The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.The results are stored in Vx.See instruction 8xy2 for more information on AND.
			cpu.v[nibble2] = (rand() % 255) & nibble34;
			cpu.programCounter += 2;
			break;
		}
		case 0xD:
		{
			// Dxyn - DRW Vx, Vy, nibble
			// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			// Read n bytes from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
			u8 x = cpu.v[nibble2];
			u8 y = cpu.v[nibble3];
			u8 h = cpu.v[nibble4];
			u8 pixel;

			cpu.v[0xF] = 0;
			for (u8 yline = 0; yline < h; yline++)
			{
				pixel = cpu.memory[cpu.i + yline];
				for (u8 xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (cpu.graphics[(x + xline + ((y + yline) * 64))] == 1)
							cpu.v[0xF] = 1;
						cpu.graphics[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
			cpu.programCounter += 2;
			break;
		}
		case 0xE:
		{
			switch (nibble3)
			{
			case 0x9:
			{
				// Ex9E - SKP Vx
				// Skip next instruction if key with the value of Vx is pressed.
				// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
				if (cpu.key[cpu.v[nibble2]] == 1)
					cpu.programCounter += 4;
				else
					cpu.programCounter += 2;
				break;
			}
			case 0xA:
			{
				//ExA1 - SKNP Vx
				//Skip next instruction if key with the value of Vx is not pressed.
				//Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
				// TODO: key press
				if (cpu.key[cpu.v[nibble2]] == 0)
					cpu.programCounter += 4;
				else
					cpu.programCounter += 2;
				break;
			}
			} break;
		}
		case 0xF:
		{
			switch (nibble4)
			{
			case 0x7:
			{
				// Fx07 - LD Vx, DT
				// Set Vx = delay timer value.
				// The value of DT is placed into Vx.
				cpu.v[nibble2] = cpu.delayTimer;
				cpu.programCounter += 2;
				break;
			}
			case 0xA:
			{
				// Fx0A - LD Vx, K
				// Wait for a key press, store the value of the key in Vx.
				// All execution stops until a key is pressed, then the value of that key is stored in Vx.
				for (u8 i = 0; i < 16; i++)
				{
					if (cpu.key[i] == 1)
					{
						cpu.programCounter += 2;
						break;
					}
				}
				break;
			}
			case 0x5:
			{
				switch (nibble3)
				{
				case 0x1:
				{
					// Fx15 - LD DT, Vx
					// Set delay timer = Vx.
					// DT is set equal to the value of Vx.
					cpu.delayTimer = cpu.v[nibble2];
					cpu.programCounter += 2;

					break;
				}
				case 0x5:
				{
					// Fx55 - LD [I], Vx
					// Store registers V0 through Vx in memory starting at location I.
					// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.

					for (u8 i = 0; i < nibble2; i++)
					{
						cpu.memory[cpu.i + i] = cpu.v[i];
					}
					cpu.programCounter += 2;
					break;
				}
				case 0x6:
				{
					// Fx65 - LD Vx, [I]
					// Read registers V0 through Vx from memory starting at location I.
					// The interpreter reads values from memory starting at location I into registers V0 through Vx.
					for (u8 i = 0; i < nibble2 + 1; i++)
					{
						cpu.v[cpu.i + i] = cpu.memory[i];
					}

					cpu.programCounter += 2;
					break;
				}
				}
				break;
			}
			case 0x8:
			{
				// Fx18 - LD ST, Vx
				// Set sound timer = Vx.
				// ST is set equal to the value of Vx.
				cpu.soundTimer = cpu.v[nibble2];
				cpu.programCounter += 2;
				break;
			}
			case 0xE:
			{
				// Fx1E - ADD I, Vx
				// Set I = I + Vx.
				// The values of I and Vx are added, and the results are stored in I.
				cpu.i = cpu.i + cpu.v[nibble2];
				cpu.programCounter += 2;
				break;
			}
			case 0x9:
			{
				// Fx29 - LD F, Vx
				// Set I = location of sprite for digit Vx.
				// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx
				cpu.i = nibble2 * 5;
				cpu.programCounter += 2;
				break;
			}
			case 0x3:
			{
				// Fx33 - LD B, Vx
				// Store BCD representation of Vx in memory locations I, I + 1, and I + 2.
				// The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I + 1, and the ones digit at location I + 2.
				u8 units = cpu.v[nibble2] % 10;
				u8 tens = cpu.v[nibble2] / 10 % 10;
				u8 hundreds = cpu.v[nibble2] / 100 % 10;

				cpu.v[cpu.i] = hundreds;
				cpu.v[cpu.i + 1] = tens;
				cpu.v[cpu.i + 2] = units;

				cpu.programCounter += 2;
				break;
			}
			default:
			{
				// TODO: Implement error handling with Propitious! -- Woop
				MessageBoxA(nullptr, "Unknown Opcode", "Fatal Error", MB_OK);
				std::exit(-1);
				break;
			}
			} break;
		}
		}
	}
}
#endif