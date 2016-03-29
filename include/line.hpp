#ifndef LINE_HPP
#define LINE_HPP

#include <string>
#include "token.hpp"
#include "addressing_mode.hpp"

struct line
{
	token label,
		  opcode,
		  operands;
	int number;

	line():number(0) {}

	line(token label, token opcode, token operands, int number)
	{
		this->label = label;
		this->opcode = opcode;
		this->operands = operands;
		this->number = number;
	}

	line(std::string label, std::string opcode, std::string operands, int number)
	{
		this->label = token(label);
		this->opcode = token(opcode);
		this->operands = token(operands);
		this->number = number;
	}

	virtual std::string to_string()
	{
		return label.value + ' ' + opcode.value + ' ' + operands.value;
	}
};

struct pass2_line : public line 
{
	int address;
	AddressingMode am;
	
	pass2_line():line(),address(0) {}
	pass2_line(line l, int address, AddressingMode am):line(l.label, l.opcode, l.operands, l.number) {
		this->address = address;
		this->am = am;
	}
	pass2_line(token label, token opcode, token operands, int number, int address, AddressingMode am)
		:line(label, opcode, operands, number) {
		this->address = address;
		this->am = am;
	}

	pass2_line(std::string label, std::string opcode, std::string operands, int number, int address, AddressingMode am)
		:line(label, opcode, operands, number) {
		this->address = address;
		this->am = am;
	}
	
	virtual std::string to_string()
	{
		return std::to_string(this->address) + line::to_string();
	}

};

#endif
