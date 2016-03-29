#ifndef ADDRESSING_MODE_HPP
#define ADDRESSING_MODE_HPP

enum AddressingMode : unsigned char
{
	Direct = 0b110000,            // disp
	DirectExtended = 0b110001,    // addr
	RelativePC = 0b110010,        // (PC) + disp
	RelativeBase = 0b110100,      // (B) + disp
	Indexed = 0b111000,           // disp + (X)
	IndexedExtended = 0b111001,   // addr + (X)
	IndexedPC = 0b111010,         // (PC) + disp + (X)
	IndexedBase = 0b111100,       // (B) + disp + (X)
	Simple = 0b000,               // b/p/e/disp
	SimpleIndexed = 0b001,        // b/p/e/disp + (X)
	Indirect = 0b100000,          // disp
	IndirectExtended = 0b100001,  // addr
	IndirectPC = 0b100010,        // (PC) + disp
	IndirectBase = 0b100100,      // (B) + disp
	Immediate = 0b010000,         // disp
	ImmediateExtended = 0b010001, // addr
	ImmediatePlusPC = 0b010010,   // (PC) + disp
	ImmediatePlusBase = 0b010100, // (B) + disp
	
	UnknownAM,
};
#endif
