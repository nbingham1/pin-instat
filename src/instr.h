#pragma once

#include <db/cached_keystore.h>
#include <stdint.h>

struct memory_key_t {
	memory_key_t();
	memory_key_t(uint64_t instr, uint64_t mem);
	~memory_key_t();

	uint64_t instr, mem;
};

struct memory_value_t {
	memory_value_t();
	memory_value_t(uint64_t read, uint64_t write);
	~memory_value_t();

	uint64_t read, write;
};

bool operator<(memory_key_t k0, memory_key_t k1);
bool operator>(memory_key_t k0, memory_key_t k1);
bool operator<=(memory_key_t k0, memory_key_t k1);
bool operator>=(memory_key_t k0, memory_key_t k1);
bool operator==(memory_key_t k0, memory_key_t k1);
bool operator!=(memory_key_t k0, memory_key_t k1);

bool operator<(memory_value_t k0, memory_value_t k1);
bool operator>(memory_value_t k0, memory_value_t k1);
bool operator<=(memory_value_t k0, memory_value_t k1);
bool operator>=(memory_value_t k0, memory_value_t k1);
bool operator==(memory_value_t k0, memory_value_t k1);
bool operator!=(memory_value_t k0, memory_value_t k1);

uint64_t update(uint64_t a, uint64_t b);
memory_value_t update_mem(memory_value_t a, memory_value_t b);

struct instruction_record_t {
	instruction_record_t();
	instruction_record_t(const char *mem, const char *exe);
	~instruction_record_t();

	core::cached_keystore<memory_key_t, memory_value_t, 5000, &update_mem> memory;
	// indexed by instruction address as obtained by INS_Address()
	core::cached_keystore<uint64_t, uint64_t, 5000, &update> execution;	
	uint64_t total;

	void finish(FILE *log = stdout);
	void exec(uint64_t instr, FILE *log = stdout);
	void read(uint64_t instr, uint64_t mem, FILE *log = stdout);
	void write(uint64_t instr, uint64_t mem, FILE *log = stdout);
};

struct assembly_t
{
	assembly_t();
	assembly_t(uint32_t opcode, const char *str);
	~assembly_t();

	uint32_t opcode;
	uint8_t category;
	char str[256];
};

assembly_t update_assembly(assembly_t a, assembly_t b);

