#pragma once

#include "cached_map.h"
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

uint64_t update(uint64_t a, uint64_t b);
memory_value_t update_mem(memory_value_t a, memory_value_t b);

struct instruction_record_t {
	instruction_record_t();
	instruction_record_t(const char *mem, const char *exe);
	~instruction_record_t();

	cached_map<memory_key_t, memory_value_t, 5000, &update_mem> memory;
	// indexed by instruction address as obtained by INS_Address()
	cached_map<uint64_t, uint64_t, 5000, &update> execution;	
	uint64_t total;

	void finish(FILE *log = stdout);
	void exec(uint64_t instr, FILE *log = stdout);
	void read(uint64_t instr, uint64_t mem, FILE *log = stdout);
	void write(uint64_t instr, uint64_t mem, FILE *log = stdout);
};

