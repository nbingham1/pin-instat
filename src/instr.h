#pragma once

#include "cached_map.h"

struct memory_key_t {
	memory_key_t();
	memory_key_t(UINT64 instr, UINT64 mem);
	~memory_key_t();

	UINT64 instr, mem;
};

struct memory_value_t {
	memory_value_t();
	memory_value_t(UINT64 read, UINT64 write);
	~memory_value_t();

	UINT64 read, write;
};

bool operator<(memory_key_t k0, memory_key_t k1);
bool operator>(memory_key_t k0, memory_key_t k1);
bool operator<=(memory_key_t k0, memory_key_t k1);
bool operator>=(memory_key_t k0, memory_key_t k1);
bool operator==(memory_key_t k0, memory_key_t k1);
bool operator!=(memory_key_t k0, memory_key_t k1);

UINT64 update(UINT64 a, UINT64 b);
memory_value_t update_mem(memory_value_t a, memory_value_t b);

struct instruction_record_t {
	instruction_record_t();
	instruction_record_t(const char *mem, const char *exe);
	~instruction_record_t();

	cached_map<memory_key_t, memory_value_t, 5000, &update_mem> memory;
	// indexed by instruction address as obtained by INS_Address()
	cached_map<UINT64, UINT64, 5000, &update> execution;	
	UINT64 total;

	void finish(FILE *log = stdout);
	void exec(UINT64 instr, FILE *log = stdout);
	void read(UINT64 instr, UINT64 mem, FILE *log = stdout);
	void write(UINT64 instr, UINT64 mem, FILE *log = stdout);
};

