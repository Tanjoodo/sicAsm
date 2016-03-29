#include "../include/line.hpp"
#include "../include/addressing_mode.hpp"
#include "../include/int24.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <cstdlib>

struct instruction
{
	unsigned char machine_opcode;
	int format;

	instruction():format(0) {}
	instruction(unsigned char machine_opcode, int format) {
		this->machine_opcode = machine_opcode;
		this->format = format;
	}
};

struct symbol
{
	int location,
		line_found;
	symbol():location(0), line_found(0) {}

	symbol(int location, int line_found) {
		this->location = location;
		this->line_found = line_found;
	}
};
std::map<std::string, instruction> instructions;
std::map<std::string, int> registers;

std::vector<std::string> split_line(std::string line);
std::vector<std::string> read_file_into_lines(char * path);
std::vector<line> parse_file(char * path);
void initialize();

template<typename T1, typename T2> bool in_map(std::map<T1, T2>& m, T1 key)
{
	typename std::map<T1, T2>::iterator search_iterator = m.find(key);
	return search_iterator != m.end();
}

inline bool is_integer(const std::string & s) { // http://stackoverflow.com/a/2845275
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char * p;
   strtol(s.c_str(), &p, 10);

   return (*p == 0);
}

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		std::cout << "Enter file name please." << std::endl;
		return 1;
	}

	initialize();

	std::vector<line> lines = parse_file(argv[1]);
	std::vector<pass2_line> pass2_lines;
	int location_counter = 0;
	int starting_address = 0;
	int program_length = 0;
	int prev_location_counter = location_counter;

	if (lines[0].opcode.value == "START") {
		int iop = std::stoi(lines[0].operands.value, 0, 16);
		location_counter = iop;
		starting_address = iop;
	}

	int i = 0;
	std::map<std::string, symbol> symbol_table;

	while (lines[i].opcode.value != "END") {
		int prev_location_counter = location_counter;
		AddressingMode am = UnknownAM;
		if (lines[i].label.value != "") {
			if (in_map<std::string, symbol>(symbol_table, lines[i].label.value)) {
				symbol offending_symbol = symbol_table[lines[i].label.value];
				std::cout <<"Error on line " << i + 1
					<<  ": duplicate symbol definition: \""
				   	<< lines[i].label.value 
					<< "\". First defined on line "
					<< symbol_table[lines[i].label.value].line_found << std::endl;
				return 2;
			} else {
				symbol_table[lines[i].label.value] = symbol(location_counter, i + 1);
			}
		}
		if (in_map<std::string, instruction>(instructions, lines[i].opcode.value)) {
			instruction ins = instructions[lines[i].opcode.value];
			location_counter += ins.format;
			if (lines[i].opcode.value == "WORD") {
				location_counter += 3;
			} else if (lines[i].opcode.value == "RESW") {
				location_counter += 3 * std::stoi(lines[i].operands.value);
			} else if (lines[i].opcode.value == "RESB") {
				location_counter += std::stoi(lines[i].operands.value);
			} else if (lines[i].opcode.value == "BYTE") {
				if (lines[i].operands.value[0] == 'X') {
					location_counter += (lines[i].operands.value.size() - 3) / 2;
				} else if (lines[i].opcode.value[0] == 'C') {
					location_counter += lines[i].operands.value.size() - 3;
				}
			}
		} else {
			std::cout << "Error on line " 
				<< i + 1
				<< ": unrecognized instruction: \"" 
				<< lines[i].opcode.value
				<< "\"." << std::endl;
			return 3;
		}
		if (in_map<std::string, instruction>(instructions, lines[i].opcode.value)) {
				std::cout << lines[i].operands.value[0] << std::endl;
				if (instructions[lines[i].opcode.value].format == 3) {
					if (lines[i].operands.value[0] == '@'){
							am = Indirect;
					} else if (lines[i].operands.value[0] == '#') {
							std::cout << "is immediate" << std::endl;
							am = Immediate;
					} else if (lines[i].operands.value.length() > 3 
							&& lines[i].operands.value.substr(lines[i].operands.value.length() - 2) == ",X") {
						am = Indexed;
					} else {
					am = UnknownAM; // Could be Direct, RelativePC or RelativeBase. We differ this decision to
					}				// the memory address.
				}
				else if (instructions[lines[i].opcode.value].format == 4) {
					if (lines[i].operands.value[0] == '@') {
						am = IndirectExtended;
					} else if (lines[i].operands.value[0] == '#') {
						am = ImmediateExtended;
					} else if (lines[i].operands.value.length() > 3
							&& lines[i].operands.value.substr(lines[i].operands.value.length() - 2) == ",X") {
						am = IndexedExtended;
					} else {
						am = DirectExtended;
					}
				}
		} // Format 1 and 2 don't have any addressing involved. It is always UnknownAM. 
		std::cout << (am == Immediate) << std::endl;
		pass2_lines.push_back(pass2_line(lines[i], prev_location_counter, am));
		++i;
	}

	pass2_lines.push_back(pass2_line(lines[i], prev_location_counter, UnknownAM));
	program_length = location_counter - starting_address;
	for (auto l: pass2_lines)
		std::cout << l.to_string() << std::endl;
	std::cout << "program length: " << program_length << std::endl;

	// ------------------------------------------------------
	// end of pass 1
	// ------------------------------------------------------

	i = 0;
	std::ofstream of;
	of.open("a.bin", std::ios::binary | std::ios::out);
	int24 buffer(0);
	unsigned char buffer_array[3] = { 0, 0, 0 };

	if (lines[0].opcode.value == "START") {
		++i;
	}

	// write H for header record and HELLO\0 for name
	of << 'H' << 'H' << 'E' << 'L' << 'L' << 'O' << '\0';
	// write starting address as 3 bytes
	buffer = starting_address;
	buffer.store_into((unsigned char *)&buffer_array);
	of << buffer_array[0] << buffer_array[1] << buffer_array[2];
	// write program length as 3 bytes;
	buffer = program_length;
	buffer.store_into((unsigned char *)&buffer_array);
	of << buffer_array[0] << buffer_array[1] << buffer_array[2];

	int current_operands = 0;
	while (pass2_lines[i].opcode.value != "END") {
			if (pass2_lines[i].am == Immediate || pass2_lines[i].am == Indirect
					|| pass2_lines[i].am == ImmediateExtended || pass2_lines[i].am == IndirectExtended) {
				std::string raw_operand = pass2_lines[i].operands.value.substr(1);
				if (is_integer(raw_operand)) {
					current_operands = std::stoi(raw_operand);
				} else {
					if (in_map<std::string, symbol>(symbol_table, raw_operand)) {
						current_operands = symbol_table[raw_operand].location;
					} else {
						std::cout << "Error on line " 
						<< pass2_lines[i].number
						<< ". Undefined label \""
						<< raw_operand << "\"." << std::endl;
						return 4;
					}
				}
			} else if (pass2_lines[i].am == Indexed || pass2_lines[i].am == IndexedExtended) {
				std::string raw_operand = 
					pass2_lines[i].operands.value.substr(0, pass2_lines[i].operands.value.length() - 2);
				if (is_integer(raw_operand)) {
					current_operands = std::stoi(raw_operand);
				} else {
					if (in_map<std::string, symbol>(symbol_table, raw_operand)) {
						current_operands = symbol_table[raw_operand].location;
					} else {
						std::cout << "Error on line " 
							<< pass2_lines[i].number
							<< ". Undefined label \""
							<< raw_operand << "\"." << std::endl;
						return 4;
					}
				}
			} else { // whole operands.value string should be the operand
				std::string raw_operand = pass2_lines[i].operands.value;
				if (instructions[pass2_lines[i].opcode.value].format == 1) {
					current_operands = 0;
				} else if (instructions[pass2_lines[i].opcode.value].format == 2) {
					int loc = raw_operand.find(',');

					int r1 = 0, r2 = 0;
					if (loc == std::string::npos) { // just one register
						if (in_map<std::string, int>(registers, raw_operand)) {
							r1 = registers[raw_operand];
						} else {
							std::cout << "Error on line " << pass2_lines[i].number
								<< ". Unrecognized register mnemonic: " << raw_operand << std::endl;
							return 5;
						}
					} else {
						if (in_map<std::string, int>(registers, raw_operand.substr(0, loc))) {
							r1 = registers[raw_operand.substr(0, loc)];
						} else { 
							std::cout << "Error on line " << pass2_lines[i].number
								<< ". Unrecognized register mnemonic: " << raw_operand.substr(0, loc)
							   	<< std::endl;
							return 5;
						}
						if (in_map<std::string, int>(registers, raw_operand.substr(loc + 1))) {
							r2 = registers[raw_operand.substr(loc + 1)];
						} else {
							std::cout << "Error on line " << pass2_lines[i].number
								<< ". Unrecognized register mnemonic: " << raw_operand.substr(loc + 1)
							   	<< std::endl;
							return 5;
						}

						current_operands |= (r1 << 4);
						current_operands |= r2;
					}
				} else if (pass2_lines[i].am == DirectExtended) { 
				}
				
			}
			++i;
	}
	return 0;
}

void initialize()
{
	instructions["ADD"]    = instruction(0x18, 3);
	instructions["ADDF"]   = instruction(0x58, 3);
	instructions["ADDR"]   = instruction(0x90, 2);
	instructions["AND"]    = instruction(0x40, 3);
	instructions["CLEAR"]  = instruction(0xB4, 2);
	instructions["COMP"]   = instruction(0x28, 3);
	instructions["COMPF"]  = instruction(0x88, 3);
	instructions["COMPR"]  = instruction(0xA0, 2);
	instructions["DIV"]    = instruction(0x24, 3);
	instructions["DIVF"]   = instruction(0x64, 3);
	instructions["DIVR"]   = instruction(0x9C, 2);
	instructions["FIX"]    = instruction(0xC4, 1);
	instructions["FLOAT"]  = instruction(0xC0, 1);
	instructions["HIO"]    = instruction(0xF4, 1);
	instructions["J"]      = instruction(0x3C, 3);
	instructions["JEQ"]    = instruction(0x30, 3);
	instructions["JGT"]    = instruction(0x34, 3);
	instructions["JLT"]    = instruction(0x38, 3);
	instructions["JSUB"]   = instruction(0x48, 3);
	instructions["LDA"]    = instruction(0x00, 3);
	instructions["LDB"]    = instruction(0x68, 3);
	instructions["LDCH"]   = instruction(0x50, 3);
	instructions["LDF"]    = instruction(0x70, 3);
	instructions["LDL"]    = instruction(0x08, 3);
	instructions["LDS"]    = instruction(0x6C, 3);
	instructions["LDT"]    = instruction(0x74, 3);
	instructions["LDX"]    = instruction(0x04, 3);
	instructions["LPS"]    = instruction(0xD0, 3);
	instructions["MUL"]    = instruction(0x20, 3);
	instructions["MULF"]   = instruction(0x60, 3);
	instructions["MULR"]   = instruction(0x98, 2);
	instructions["NORM"]   = instruction(0xC8, 1);
	instructions["OR"]     = instruction(0x44, 3);
	instructions["RD"]     = instruction(0xD8, 3);
	instructions["RMO"]    = instruction(0xAC, 2);
	instructions["RSUB"]   = instruction(0x4C, 3);
	instructions["SHIFTL"] = instruction(0xA4, 2);
	instructions["SHIFTR"] = instruction(0xA8, 2);
	instructions["SIO"]    = instruction(0xF0, 1);
	instructions["SSK"]    = instruction(0xEC, 3);
	instructions["STA"]    = instruction(0x0C, 3);
	instructions["STB"]    = instruction(0x78, 3);
	instructions["STCH"]   = instruction(0x54, 3);
	instructions["STF"]    = instruction(0x80, 3);
	instructions["STI"]    = instruction(0xD4, 3);
	instructions["STL"]    = instruction(0x14, 3);
	instructions["STS"]    = instruction(0x7C, 3);
	instructions["STSW"]   = instruction(0xE8, 3);
	instructions["STT"]    = instruction(0x10, 3);
	instructions["SUB"]    = instruction(0x1C, 3);
	instructions["SUBF"]   = instruction(0x5C, 3);
	instructions["SUBR"]   = instruction(0x94, 2);
	instructions["SVC"]    = instruction(0xB0, 2);
	instructions["TD"]     = instruction(0xE0, 3);
	instructions["TIO"]    = instruction(0xF8, 1);
	instructions["TIX"]    = instruction(0x2C, 3);
	instructions["TIXR"]   = instruction(0xB8, 2);
	instructions["WD"]     = instruction(0xDC, 3);

	instructions["+ADD"]    = instruction(0x18, 4);
	instructions["+ADDF"]   = instruction(0x58, 4);
	instructions["+AND"]    = instruction(0x40, 4);
	instructions["+COMP"]   = instruction(0x28, 4);
	instructions["+COMPF"]  = instruction(0x88, 4);
	instructions["+DIV"]    = instruction(0x24, 4);
	instructions["+DIVF"]   = instruction(0x64, 4);
	instructions["+DIVR"]   = instruction(0x9C, 2);
	instructions["+J"]      = instruction(0x3C, 4);
	instructions["+JEQ"]    = instruction(0x30, 4);
	instructions["+JGT"]    = instruction(0x34, 4);
	instructions["+JLT"]    = instruction(0x38, 4);
	instructions["+JSUB"]   = instruction(0x48, 4);
	instructions["+LDA"]    = instruction(0x00, 4);
	instructions["+LDB"]    = instruction(0x68, 4);
	instructions["+LDCH"]   = instruction(0x50, 4);
	instructions["+LDF"]    = instruction(0x70, 4);
	instructions["+LDL"]    = instruction(0x08, 4);
	instructions["+LDS"]    = instruction(0x6C, 4);
	instructions["+LDT"]    = instruction(0x74, 4);
	instructions["+LDX"]    = instruction(0x04, 4);
	instructions["+LPS"]    = instruction(0xD0, 4);
	instructions["+MUL"]    = instruction(0x20, 4);
	instructions["+MULF"]   = instruction(0x60, 4);
	instructions["+OR"]     = instruction(0x44, 4);
	instructions["+RD"]     = instruction(0xD8, 4);
	instructions["+RSUB"]   = instruction(0x4C, 4);
	instructions["+SSK"]    = instruction(0xEC, 4);
	instructions["+STA"]    = instruction(0x0C, 4);
	instructions["+STB"]    = instruction(0x78, 4);
	instructions["+STCH"]   = instruction(0x54, 4);
	instructions["+STF"]    = instruction(0x80, 4);
	instructions["+STI"]    = instruction(0xD4, 4);
	instructions["+STL"]    = instruction(0x14, 4);
	instructions["+STS"]    = instruction(0x7C, 4);
	instructions["+STSW"]   = instruction(0xE8, 4);
	instructions["+STT"]    = instruction(0x10, 4);
	instructions["+SUB"]    = instruction(0x1C, 4);
	instructions["+SUBF"]   = instruction(0x5C, 4);
	instructions["+TD"]     = instruction(0xE0, 4);
	instructions["+TIX"]    = instruction(0x2C, 4);
	instructions["+WD"]     = instruction(0xDC, 4);


	instructions["START"]    = instruction(0xFF, 0);
	instructions["BASE"]     = instruction(0xFF, 0);
	instructions["END"]      = instruction(0xFF, 0);
	instructions["WORD"]     = instruction(0xFF, 0);
	instructions["RESW"]     = instruction(0xFF, 0);
	instructions["RESB"]     = instruction(0xFF, 0);
	instructions["BYTE"]     = instruction(0xFF, 0);

	registers["A"]  = 0;
	registers["X"]  = 1;
	registers["L"]  = 2;
	registers["PC"] = 8;
	registers["SW"] = 9;
	registers["B"]  = 3;
	registers["S"]  = 4;
	registers["T"]  = 5;
	registers["F"]  = 6;
}

std::vector<line> parse_file(char * path)
{
	std::vector<std::string> lines = read_file_into_lines(path);
	std::vector<line> result;

	for (int i = 0; i < lines.size(); ++i) {
		if (lines[i][0] != '.') { // ignore comments. Comments start with a . and take the whole line.
			std::vector<std::string> split_string = split_line(lines[i]);
			if (split_string.size() == 1) {
				result.push_back(line("", split_string[0], "", i + 1));
			} else if (split_string.size() == 2) {
				if (in_map<std::string, instruction>(instructions, split_string[0])) { // opcode found
					result.push_back(line("", split_string[0], split_string[1], i + 1));
				} else {
					result.push_back(line(split_string[0], split_string[1], "", i + 1));
				}
			} else if (split_string.size() == 3) {
				result.push_back(line(split_string[0], split_string[1],
							split_string[2], i + 1));
			} else {
				if (split_string.size() != 0) { // empty lines are ok
					std::cout << "Error: Malformed line. Line number: " 
						<< i + 1
						<< " expected 1, 2 or 3 tokens. Got " << split_string.size()
						<< std::endl;
				}
			}
		}
	}

	return result;
}

std::vector<std::string> read_file_into_lines(char * path)
{
	std::ifstream input_file;
	input_file.open(path);
	std::vector<std::string> lines;
	for (std::string line; std::getline(input_file, line);)
	{
		lines.push_back(line);
	}

	return lines;
}

std::vector<std::string> split_line(std::string line)
{
	using namespace std;

	vector<string> results;

	string current;

	for (auto c: line) {
		if (c == ' ' || c == '\n' || c == '\t') {
			if (current != "") {
				results.push_back(current);
			}
			current = "";
		} else {
			if (c != '\r') current += c;
		}
	}

	if (current != "")
		results.push_back(current);

	return results;
}
